use crate::api::{client_key_header, failure_to_ise, ServerState, SNAPSHOT_CONTENT_TYPE};
use crate::server::{add_snapshot, VersionId, NIL_VERSION_ID};
use actix_web::{error, post, web, HttpMessage, HttpRequest, HttpResponse, Result};
use futures::StreamExt;
use std::sync::Arc;

/// Max snapshot size: 100MB
const MAX_SIZE: usize = 100 * 1024 * 1024;

/// Add a new snapshot, after checking prerequisites.  The snapshot should be transmitted in the
/// request entity body and must have content-type `application/vnd.taskchampion.snapshot`.  The
/// content can be encoded in any of the formats supported by actix-web.
///
/// On success, the response is a 200 OK. Even in a 200 OK, the snapshot may not appear in a
/// subsequent `GetSnapshot` call.
///
/// Returns other 4xx or 5xx responses on other errors.
#[post("/v1/client/add-snapshot/{version_id}")]
pub(crate) async fn service(
    req: HttpRequest,
    server_state: web::Data<Arc<ServerState>>,
    web::Path((version_id,)): web::Path<(VersionId,)>,
    mut payload: web::Payload,
) -> Result<HttpResponse> {
    // check content-type
    if req.content_type() != SNAPSHOT_CONTENT_TYPE {
        return Err(error::ErrorBadRequest("Bad content-type"));
    }

    let client_key = client_key_header(&req)?;

    // read the body in its entirety
    let mut body = web::BytesMut::new();
    while let Some(chunk) = payload.next().await {
        let chunk = chunk?;
        // limit max size of in-memory payload
        if (body.len() + chunk.len()) > MAX_SIZE {
            return Err(error::ErrorBadRequest("Snapshot over maximum allowed size"));
        }
        body.extend_from_slice(&chunk);
    }

    if body.is_empty() {
        return Err(error::ErrorBadRequest("No snapshot supplied"));
    }

    // note that we do not open the transaction until the body has been read
    // completely, to avoid blocking other storage access while that data is
    // in transit.
    let mut txn = server_state.storage.txn().map_err(failure_to_ise)?;

    // get, or create, the client
    let client = match txn.get_client(client_key).map_err(failure_to_ise)? {
        Some(client) => client,
        None => {
            txn.new_client(client_key, NIL_VERSION_ID)
                .map_err(failure_to_ise)?;
            txn.get_client(client_key).map_err(failure_to_ise)?.unwrap()
        }
    };

    add_snapshot(
        txn,
        &server_state.config,
        client_key,
        client,
        version_id,
        body.to_vec(),
    )
    .map_err(failure_to_ise)?;
    Ok(HttpResponse::Ok().body(""))
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::storage::{InMemoryStorage, Storage};
    use crate::Server;
    use actix_web::{http::StatusCode, test, App};
    use pretty_assertions::assert_eq;
    use uuid::Uuid;

    #[actix_rt::test]
    async fn test_success() -> anyhow::Result<()> {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, version_id).unwrap();
            txn.add_version(client_key, version_id, NIL_VERSION_ID, vec![])?;
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-snapshot/{}", version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header("Content-Type", "application/vnd.taskchampion.snapshot")
            .header("X-Client-Key", client_key.to_string())
            .set_payload(b"abcd".to_vec())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);

        // read back that snapshot
        let uri = "/v1/client/snapshot";
        let req = test::TestRequest::get()
            .uri(uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let mut resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);

        use futures::StreamExt;
        let (bytes, _) = resp.take_body().into_future().await;
        assert_eq!(bytes.unwrap().unwrap().as_ref(), b"abcd");

        Ok(())
    }

    #[actix_rt::test]
    async fn test_not_added_200() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, NIL_VERSION_ID).unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        // add a snapshot for a nonexistent version
        let uri = format!("/v1/client/add-snapshot/{}", version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header("Content-Type", "application/vnd.taskchampion.snapshot")
            .header("X-Client-Key", client_key.to_string())
            .set_payload(b"abcd".to_vec())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);

        // read back, seeing no snapshot
        let uri = "/v1/client/snapshot";
        let req = test::TestRequest::get()
            .uri(uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::NOT_FOUND);
    }

    #[actix_rt::test]
    async fn test_bad_content_type() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-snapshot/{}", version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header("Content-Type", "not/correct")
            .header("X-Client-Key", client_key.to_string())
            .set_payload(b"abcd".to_vec())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::BAD_REQUEST);
    }

    #[actix_rt::test]
    async fn test_empty_body() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-snapshot/{}", version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header(
                "Content-Type",
                "application/vnd.taskchampion.history-segment",
            )
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::BAD_REQUEST);
    }
}

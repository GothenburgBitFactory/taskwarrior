use crate::api::{
    client_key_header, failure_to_ise, ServerState, HISTORY_SEGMENT_CONTENT_TYPE,
    PARENT_VERSION_ID_HEADER, SNAPSHOT_REQUEST_HEADER, VERSION_ID_HEADER,
};
use crate::server::{add_version, AddVersionResult, SnapshotUrgency, VersionId, NIL_VERSION_ID};
use actix_web::{error, post, web, HttpMessage, HttpRequest, HttpResponse, Result};
use futures::StreamExt;
use std::sync::Arc;

/// Max history segment size: 100MB
const MAX_SIZE: usize = 100 * 1024 * 1024;

/// Add a new version, after checking prerequisites.  The history segment should be transmitted in
/// the request entity body and must have content-type
/// `application/vnd.taskchampion.history-segment`.  The content can be encoded in any of the
/// formats supported by actix-web.
///
/// On success, the response is a 200 OK with the new version ID in the `X-Version-Id` header.  If
/// the version cannot be added due to a conflict, the response is a 409 CONFLICT with the expected
/// parent version ID in the `X-Parent-Version-Id` header.
///
/// If included, a snapshot request appears in the `X-Snapshot-Request` header with value
/// `urgency=low` or `urgency=high`.
///
/// Returns other 4xx or 5xx responses on other errors.
#[post("/v1/client/add-version/{parent_version_id}")]
pub(crate) async fn service(
    req: HttpRequest,
    server_state: web::Data<Arc<ServerState>>,
    web::Path((parent_version_id,)): web::Path<(VersionId,)>,
    mut payload: web::Payload,
) -> Result<HttpResponse> {
    // check content-type
    if req.content_type() != HISTORY_SEGMENT_CONTENT_TYPE {
        return Err(error::ErrorBadRequest("Bad content-type"));
    }

    let client_key = client_key_header(&req)?;

    // read the body in its entirety
    let mut body = web::BytesMut::new();
    while let Some(chunk) = payload.next().await {
        let chunk = chunk?;
        // limit max size of in-memory payload
        if (body.len() + chunk.len()) > MAX_SIZE {
            return Err(error::ErrorBadRequest("overflow"));
        }
        body.extend_from_slice(&chunk);
    }

    if body.is_empty() {
        return Err(error::ErrorBadRequest("Empty body"));
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

    let (result, snap_urgency) = add_version(
        txn,
        &server_state.config,
        client_key,
        client,
        parent_version_id,
        body.to_vec(),
    )
    .map_err(failure_to_ise)?;

    Ok(match result {
        AddVersionResult::Ok(version_id) => {
            let mut rb = HttpResponse::Ok();
            rb.header(VERSION_ID_HEADER, version_id.to_string());
            match snap_urgency {
                SnapshotUrgency::None => {}
                SnapshotUrgency::Low => {
                    rb.header(SNAPSHOT_REQUEST_HEADER, "urgency=low");
                }
                SnapshotUrgency::High => {
                    rb.header(SNAPSHOT_REQUEST_HEADER, "urgency=high");
                }
            };
            rb.finish()
        }
        AddVersionResult::ExpectedParentVersion(parent_version_id) => {
            let mut rb = HttpResponse::Conflict();
            rb.header(PARENT_VERSION_ID_HEADER, parent_version_id.to_string());
            rb.finish()
        }
    })
}

#[cfg(test)]
mod test {
    use crate::storage::{InMemoryStorage, Storage};
    use crate::Server;
    use actix_web::{http::StatusCode, test, App};
    use pretty_assertions::assert_eq;
    use uuid::Uuid;

    #[actix_rt::test]
    async fn test_success() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, Uuid::nil()).unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-version/{}", parent_version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header(
                "Content-Type",
                "application/vnd.taskchampion.history-segment",
            )
            .header("X-Client-Key", client_key.to_string())
            .set_payload(b"abcd".to_vec())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);

        // the returned version ID is random, but let's check that it's not
        // the passed parent version ID, at least
        let new_version_id = resp.headers().get("X-Version-Id").unwrap();
        assert!(new_version_id != &version_id.to_string());

        // Shapshot should be requested, since there is no existing snapshot
        let snapshot_request = resp.headers().get("X-Snapshot-Request").unwrap();
        assert_eq!(snapshot_request, "urgency=high");

        assert_eq!(resp.headers().get("X-Parent-Version-Id"), None);
    }

    #[actix_rt::test]
    async fn test_conflict() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, version_id).unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-version/{}", parent_version_id);
        let req = test::TestRequest::post()
            .uri(&uri)
            .header(
                "Content-Type",
                "application/vnd.taskchampion.history-segment",
            )
            .header("X-Client-Key", client_key.to_string())
            .set_payload(b"abcd".to_vec())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::CONFLICT);
        assert_eq!(resp.headers().get("X-Version-Id"), None);
        assert_eq!(
            resp.headers().get("X-Parent-Version-Id").unwrap(),
            &version_id.to_string()
        );
    }

    #[actix_rt::test]
    async fn test_bad_content_type() {
        let client_key = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-version/{}", parent_version_id);
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
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/add-version/{}", parent_version_id);
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

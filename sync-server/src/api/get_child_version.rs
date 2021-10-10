use crate::api::{
    client_key_header, failure_to_ise, ServerState, HISTORY_SEGMENT_CONTENT_TYPE,
    PARENT_VERSION_ID_HEADER, VERSION_ID_HEADER,
};
use crate::server::{get_child_version, GetVersionResult, VersionId};
use actix_web::{error, get, web, HttpRequest, HttpResponse, Result};
use std::sync::Arc;

/// Get a child version.
///
/// On succcess, the response is the same sequence of bytes originally sent to the server,
/// with content-type `application/vnd.taskchampion.history-segment`.  The `X-Version-Id` and
/// `X-Parent-Version-Id` headers contain the corresponding values.
///
/// If no such child exists, returns a 404 with no content.
/// Returns other 4xx or 5xx responses on other errors.
#[get("/v1/client/get-child-version/{parent_version_id}")]
pub(crate) async fn service(
    req: HttpRequest,
    server_state: web::Data<Arc<ServerState>>,
    web::Path((parent_version_id,)): web::Path<(VersionId,)>,
) -> Result<HttpResponse> {
    let mut txn = server_state.storage.txn().map_err(failure_to_ise)?;

    let client_key = client_key_header(&req)?;

    let client = txn
        .get_client(client_key)
        .map_err(failure_to_ise)?
        .ok_or_else(|| error::ErrorNotFound("no such client"))?;

    return match get_child_version(
        txn,
        &server_state.config,
        client_key,
        client,
        parent_version_id,
    )
    .map_err(failure_to_ise)?
    {
        GetVersionResult::Success {
            version_id,
            parent_version_id,
            history_segment,
        } => Ok(HttpResponse::Ok()
            .content_type(HISTORY_SEGMENT_CONTENT_TYPE)
            .header(VERSION_ID_HEADER, version_id.to_string())
            .header(PARENT_VERSION_ID_HEADER, parent_version_id.to_string())
            .body(history_segment)),
        GetVersionResult::NotFound => Err(error::ErrorNotFound("no such version")),
        GetVersionResult::Gone => Err(error::ErrorGone("version has been deleted")),
    };
}

#[cfg(test)]
mod test {
    use crate::server::NIL_VERSION_ID;
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
            txn.new_client(client_key, Uuid::new_v4()).unwrap();
            txn.add_version(client_key, version_id, parent_version_id, b"abcd".to_vec())
                .unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/get-child-version/{}", parent_version_id);
        let req = test::TestRequest::get()
            .uri(&uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let mut resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);
        assert_eq!(
            resp.headers().get("X-Version-Id").unwrap(),
            &version_id.to_string()
        );
        assert_eq!(
            resp.headers().get("X-Parent-Version-Id").unwrap(),
            &parent_version_id.to_string()
        );
        assert_eq!(
            resp.headers().get("Content-Type").unwrap(),
            &"application/vnd.taskchampion.history-segment".to_string()
        );

        use futures::StreamExt;
        let (bytes, _) = resp.take_body().into_future().await;
        assert_eq!(bytes.unwrap().unwrap().as_ref(), b"abcd");
    }

    #[actix_rt::test]
    async fn test_client_not_found() {
        let client_key = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = format!("/v1/client/get-child-version/{}", parent_version_id);
        let req = test::TestRequest::get()
            .uri(&uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::NOT_FOUND);
        assert_eq!(resp.headers().get("X-Version-Id"), None);
        assert_eq!(resp.headers().get("X-Parent-Version-Id"), None);
    }

    #[actix_rt::test]
    async fn test_version_not_found_and_gone() {
        let client_key = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // create the client, but not the version
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, Uuid::new_v4()).unwrap();
        }
        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        // the child of an unknown parent_version_id is GONE
        let uri = format!("/v1/client/get-child-version/{}", parent_version_id);
        let req = test::TestRequest::get()
            .uri(&uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::GONE);
        assert_eq!(resp.headers().get("X-Version-Id"), None);
        assert_eq!(resp.headers().get("X-Parent-Version-Id"), None);

        // but the child of the nil parent_version_id is NOT FOUND, since
        // there is no snapshot.  The tests in crate::server test more
        // corner cases.
        let uri = format!("/v1/client/get-child-version/{}", NIL_VERSION_ID);
        let req = test::TestRequest::get()
            .uri(&uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::NOT_FOUND);
        assert_eq!(resp.headers().get("X-Version-Id"), None);
        assert_eq!(resp.headers().get("X-Parent-Version-Id"), None);
    }
}

use crate::api::{
    client_key_header, failure_to_ise, ServerState, SNAPSHOT_CONTENT_TYPE, VERSION_ID_HEADER,
};
use crate::server::get_snapshot;
use actix_web::{error, get, web, HttpRequest, HttpResponse, Result};
use std::sync::Arc;

/// Get a snapshot.
///
/// If a snapshot for this client exists, it is returned with content-type
/// `application/vnd.taskchampion.snapshot`.  The `X-Version-Id` header contains the version of the
/// snapshot.
///
/// If no snapshot exists, returns a 404 with no content.  Returns other 4xx or 5xx responses on
/// other errors.
#[get("/v1/client/snapshot")]
pub(crate) async fn service(
    req: HttpRequest,
    server_state: web::Data<Arc<ServerState>>,
) -> Result<HttpResponse> {
    let mut txn = server_state.storage.txn().map_err(failure_to_ise)?;

    let client_key = client_key_header(&req)?;

    let client = txn
        .get_client(client_key)
        .map_err(failure_to_ise)?
        .ok_or_else(|| error::ErrorNotFound("no such client"))?;

    if let Some((version_id, data)) =
        get_snapshot(txn, &server_state.config, client_key, client).map_err(failure_to_ise)?
    {
        Ok(HttpResponse::Ok()
            .content_type(SNAPSHOT_CONTENT_TYPE)
            .header(VERSION_ID_HEADER, version_id.to_string())
            .body(data))
    } else {
        Err(error::ErrorNotFound("no snapshot"))
    }
}

#[cfg(test)]
mod test {
    use crate::storage::{InMemoryStorage, Snapshot, Storage};
    use crate::Server;
    use actix_web::{http::StatusCode, test, App};
    use chrono::{TimeZone, Utc};
    use pretty_assertions::assert_eq;
    use uuid::Uuid;

    #[actix_rt::test]
    async fn test_not_found() {
        let client_key = Uuid::new_v4();
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, Uuid::new_v4()).unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = "/v1/client/snapshot";
        let req = test::TestRequest::get()
            .uri(uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::NOT_FOUND);
    }

    #[actix_rt::test]
    async fn test_success() {
        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let snapshot_data = vec![1, 2, 3, 4];
        let storage: Box<dyn Storage> = Box::new(InMemoryStorage::new());

        // set up the storage contents..
        {
            let mut txn = storage.txn().unwrap();
            txn.new_client(client_key, Uuid::new_v4()).unwrap();
            txn.set_snapshot(
                client_key,
                Snapshot {
                    version_id,
                    versions_since: 3,
                    timestamp: Utc.ymd(2001, 9, 9).and_hms(1, 46, 40),
                },
                snapshot_data.clone(),
            )
            .unwrap();
        }

        let server = Server::new(Default::default(), storage);
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let uri = "/v1/client/snapshot";
        let req = test::TestRequest::get()
            .uri(uri)
            .header("X-Client-Key", client_key.to_string())
            .to_request();
        let mut resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::OK);

        use futures::StreamExt;
        let (bytes, _) = resp.take_body().into_future().await;
        assert_eq!(bytes.unwrap().unwrap().as_ref(), snapshot_data);
    }
}

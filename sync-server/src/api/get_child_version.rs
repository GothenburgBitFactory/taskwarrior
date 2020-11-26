use crate::api::{
    ServerState, HISTORY_SEGMENT_CONTENT_TYPE, PARENT_VERSION_ID_HEADER, VERSION_ID_HEADER,
};
use crate::server::{ClientId, VersionId};
use actix_web::{error, get, http::StatusCode, web, HttpResponse, Result};

/// Get a child version.
///
/// On succcess, the response is the same sequence of bytes originally sent to the server,
/// with content-type `application/vnd.taskchampion.history-segment`.  The `X-Version-Id` and
/// `X-Parent-Version-Id` headers contain the corresponding values.
///
/// If no such child exists, returns a 404 with no content.
/// Returns other 4xx or 5xx responses on other errors.
#[get("/client/{client_id}/get-child-version/{parent_version_id}")]
pub(crate) async fn service(
    data: web::Data<ServerState>,
    web::Path((client_id, parent_version_id)): web::Path<(ClientId, VersionId)>,
) -> Result<HttpResponse> {
    let result = data
        .get_child_version(client_id, parent_version_id)
        .map_err(|e| error::InternalError::new(e, StatusCode::INTERNAL_SERVER_ERROR))?;
    if let Some(result) = result {
        Ok(HttpResponse::Ok()
            .content_type(HISTORY_SEGMENT_CONTENT_TYPE)
            .header(VERSION_ID_HEADER, result.version_id.to_string())
            .header(
                PARENT_VERSION_ID_HEADER,
                result.parent_version_id.to_string(),
            )
            .body(result.history_segment))
    } else {
        Err(error::ErrorNotFound("no such version"))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::api::ServerState;
    use crate::app_scope;
    use crate::server::{GetVersionResult, SyncServer};
    use crate::test::TestServer;
    use actix_web::{test, App};
    use taskchampion::Uuid;

    #[actix_rt::test]
    async fn test_success() {
        let client_id = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let server_box: Box<dyn SyncServer> = Box::new(TestServer {
            expected_client_id: client_id,
            gcv_expected_parent_version_id: parent_version_id,
            gcv_result: Some(GetVersionResult {
                version_id: version_id,
                parent_version_id: parent_version_id,
                history_segment: b"abcd".to_vec(),
            }),
            ..Default::default()
        });
        let server_state = ServerState::new(server_box);
        let mut app = test::init_service(App::new().service(app_scope(server_state))).await;

        let uri = format!(
            "/client/{}/get-child-version/{}",
            client_id, parent_version_id
        );
        let req = test::TestRequest::get().uri(&uri).to_request();
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
    async fn test_not_found() {
        let client_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let server_box: Box<dyn SyncServer> = Box::new(TestServer {
            expected_client_id: client_id,
            gcv_expected_parent_version_id: parent_version_id,
            gcv_result: None,
            ..Default::default()
        });
        let server_state = ServerState::new(server_box);
        let mut app = test::init_service(App::new().service(app_scope(server_state))).await;

        let uri = format!(
            "/client/{}/get-child-version/{}",
            client_id, parent_version_id
        );
        let req = test::TestRequest::get().uri(&uri).to_request();
        let resp = test::call_service(&mut app, req).await;
        assert_eq!(resp.status(), StatusCode::NOT_FOUND);
        assert_eq!(resp.headers().get("X-Version-Id"), None);
        assert_eq!(resp.headers().get("X-Parent-Version-Id"), None);
    }
}

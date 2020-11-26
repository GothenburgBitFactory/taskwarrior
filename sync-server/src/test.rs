use crate::api::ServerState;
use crate::app_scope;
use crate::server::{
    AddVersionResult, ClientId, GetVersionResult, HistorySegment, SyncServer, VersionId,
};
use actix_web::{test, App};
use failure::Fallible;

#[derive(Default)]
pub(crate) struct TestServer {
    /// test server will panic if this is not given
    pub expected_client_id: ClientId,

    pub gcv_expected_parent_version_id: VersionId,
    pub gcv_result: Option<GetVersionResult>,

    pub av_expected_parent_version_id: VersionId,
    pub av_expected_history_segment: HistorySegment,
    pub av_result: Option<AddVersionResult>,
}

impl SyncServer for TestServer {
    fn get_child_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
    ) -> Fallible<Option<GetVersionResult>> {
        assert_eq!(client_id, self.expected_client_id);
        assert_eq!(parent_version_id, self.gcv_expected_parent_version_id);
        Ok(self.gcv_result.clone())
    }

    fn add_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult> {
        assert_eq!(client_id, self.expected_client_id);
        assert_eq!(parent_version_id, self.av_expected_parent_version_id);
        assert_eq!(history_segment, self.av_expected_history_segment);
        Ok(self.av_result.clone().unwrap())
    }
}

#[actix_rt::test]
async fn test_index_get() {
    let server_box: Box<dyn SyncServer> = Box::new(TestServer {
        ..Default::default()
    });
    let server_state = ServerState::new(server_box);
    let mut app = test::init_service(App::new().service(app_scope(server_state))).await;

    let req = test::TestRequest::get().uri("/").to_request();
    let resp = test::call_service(&mut app, req).await;
    assert!(resp.status().is_success());
}

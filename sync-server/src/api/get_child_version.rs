use crate::api::{
    ServerState, HISTORY_SEGMENT_CONTENT_TYPE, PARENT_VERSION_ID_HEADER, VERSION_ID_HEADER,
};
use crate::types::{ClientId, VersionId};
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

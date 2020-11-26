use crate::api::{
    ServerState, HISTORY_SEGMENT_CONTENT_TYPE, PARENT_VERSION_ID_HEADER, VERSION_ID_HEADER,
};
use crate::server::{AddVersionResult, ClientId, VersionId};
use actix_web::{
    error, http::StatusCode, post, web, HttpMessage, HttpRequest, HttpResponse, Result,
};
use futures::StreamExt;

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
/// Returns other 4xx or 5xx responses on other errors.
#[post("/client/{client_id}/add-version/{parent_version_id}")]
pub(crate) async fn service(
    req: HttpRequest,
    data: web::Data<ServerState>,
    web::Path((client_id, parent_version_id)): web::Path<(ClientId, VersionId)>,
    mut payload: web::Payload,
) -> Result<HttpResponse> {
    // check content-type
    if req.content_type() != HISTORY_SEGMENT_CONTENT_TYPE {
        return Err(error::ErrorBadRequest("Bad content-type"));
    }

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

    let result = data
        .add_version(client_id, parent_version_id, body.to_vec())
        .map_err(|e| error::InternalError::new(e, StatusCode::INTERNAL_SERVER_ERROR))?;
    Ok(match result {
        AddVersionResult::Ok(version_id) => HttpResponse::Ok()
            .header(VERSION_ID_HEADER, version_id.to_string())
            .body(""),
        AddVersionResult::ExpectedParentVersion(parent_version_id) => HttpResponse::Conflict()
            .header(PARENT_VERSION_ID_HEADER, parent_version_id.to_string())
            .body(""),
    })
}

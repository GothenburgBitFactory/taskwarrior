use crate::api::ServerState;
use crate::types::{ClientId, VersionId};
use actix_web::{error, get, http::StatusCode, web, HttpResponse, Result};

#[get("/client/{client_id}/get-child-version/{parent_version_id}")]
pub(crate) async fn service(
    data: web::Data<ServerState>,
    web::Path((client_id, parent_version_id)): web::Path<(ClientId, VersionId)>,
) -> Result<HttpResponse> {
    let result = data
        .get_child_version(client_id, parent_version_id)
        .map_err(|e| error::InternalError::new(e, StatusCode::INTERNAL_SERVER_ERROR))?;
    if let Some(result) = result {
        Ok(HttpResponse::Ok().json(result))
    } else {
        Err(error::ErrorNotFound("no such version"))
    }
}

use crate::server::SyncServer;
use crate::types::{ClientId, HistorySegment, VersionId};
use actix_web::{error, http::StatusCode, post, web, HttpResponse, Responder, Result};
use serde::{Deserialize, Serialize};
use std::sync::Arc;

/// Request body to add_version
#[derive(Serialize, Deserialize)]
pub(crate) struct AddVersionRequest {
    // TODO: temporary!
    #[serde(default)]
    history_segment: HistorySegment,
}

#[post("/client/{client_id}/add-version/{parent_version_id}")]
pub(crate) async fn service(
    data: web::Data<Arc<SyncServer>>,
    web::Path((client_id, parent_version_id)): web::Path<(ClientId, VersionId)>,
    body: web::Json<AddVersionRequest>,
) -> Result<impl Responder> {
    let result = data
        .add_version(client_id, parent_version_id, &body.history_segment)
        .map_err(|e| error::InternalError::new(e, StatusCode::INTERNAL_SERVER_ERROR))?;
    Ok(HttpResponse::Ok().json(result))
}

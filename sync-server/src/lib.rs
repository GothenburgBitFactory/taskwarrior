#![deny(clippy::all)]

mod api;
mod server;
pub mod storage;

use crate::storage::Storage;
use actix_web::{get, web, Responder, Scope};
use api::{api_scope, ServerState};

#[get("/")]
async fn index() -> impl Responder {
    format!("TaskChampion sync server v{}", env!("CARGO_PKG_VERSION"))
}

/// A Server represents a sync server.
#[derive(Clone)]
pub struct Server {
    storage: ServerState,
}

impl Server {
    /// Create a new sync server with the given storage implementation.
    pub fn new(storage: Box<dyn Storage>) -> Self {
        Self {
            storage: storage.into(),
        }
    }

    /// Get an Actix-web service for this server.
    pub fn service(&self) -> Scope {
        web::scope("")
            .data(self.storage.clone())
            .service(index)
            .service(api_scope())
    }
}

#[cfg(test)]
mod test {
    pub(crate) fn init_logging() {
        let _ = env_logger::builder().is_test(true).try_init();
    }
}

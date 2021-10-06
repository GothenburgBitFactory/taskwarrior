#![deny(clippy::all)]

mod api;
mod server;
pub mod storage;

use crate::storage::Storage;
use actix_web::{get, middleware, web, Responder};
use anyhow::Context;
use api::{api_scope, ServerState};
use std::sync::Arc;

#[get("/")]
async fn index() -> impl Responder {
    format!("TaskChampion sync server v{}", env!("CARGO_PKG_VERSION"))
}

/// A Server represents a sync server.
#[derive(Clone)]
pub struct Server {
    server_state: Arc<ServerState>,
}

/// ServerConfig contains configuration parameters for the server.
pub struct ServerConfig {
    /// Target number of days between snapshots.
    pub snapshot_days: i64,

    /// Target number of versions between snapshots.
    pub snapshot_versions: u32,
}

impl Default for ServerConfig {
    fn default() -> Self {
        ServerConfig {
            snapshot_days: 14,
            snapshot_versions: 100,
        }
    }
}
impl ServerConfig {
    pub fn from_args(snapshot_days: &str, snapshot_versions: &str) -> anyhow::Result<ServerConfig> {
        Ok(ServerConfig {
            snapshot_days: snapshot_days
                .parse()
                .context("--snapshot-days must be a number")?,
            snapshot_versions: snapshot_versions
                .parse()
                .context("--snapshot-days must be a number")?,
        })
    }
}

impl Server {
    /// Create a new sync server with the given storage implementation.
    pub fn new(config: ServerConfig, storage: Box<dyn Storage>) -> Self {
        Self {
            server_state: Arc::new(ServerState { config, storage }),
        }
    }

    /// Get an Actix-web service for this server.
    pub fn config(&self, cfg: &mut web::ServiceConfig) {
        cfg.service(
            web::scope("")
                .data(self.server_state.clone())
                .wrap(
                    middleware::DefaultHeaders::new()
                        .header("Cache-Control", "no-store, max-age=0"),
                )
                .service(index)
                .service(api_scope()),
        );
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::storage::InMemoryStorage;
    use actix_web::{test, App};
    use pretty_assertions::assert_eq;

    pub(crate) fn init_logging() {
        let _ = env_logger::builder().is_test(true).try_init();
    }

    #[actix_rt::test]
    async fn test_cache_control() {
        let server = Server::new(Default::default(), Box::new(InMemoryStorage::new()));
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let req = test::TestRequest::get().uri("/").to_request();
        let resp = test::call_service(&mut app, req).await;
        assert!(resp.status().is_success());
        assert_eq!(
            resp.headers().get("Cache-Control").unwrap(),
            &"no-store, max-age=0".to_string()
        )
    }
}

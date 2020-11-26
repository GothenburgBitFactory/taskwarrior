use actix_web::{App, HttpServer};
use server::SyncServer;
use std::sync::Arc;

mod api;
mod server;
mod types;

// TODO: use hawk to sign requests

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let sync_server = Arc::new(SyncServer::new());

    HttpServer::new(move || {
        App::new()
            .data(sync_server.clone())
            .service(api::get_child_version::service)
            .service(api::add_version::service)
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}

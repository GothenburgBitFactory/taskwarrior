use actix_web::{App, HttpServer};
use std::sync::Arc;

mod api;
mod server;
mod types;

// TODO: use hawk to sign requests

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let server_state = Arc::new(Box::new(server::NullSyncServer::new()));

    HttpServer::new(move || {
        App::new()
            .data(server_state.clone())
            .service(api::get_child_version::service)
            .service(api::add_version::service)
    })
    .bind("127.0.0.1:8080")?
    .run()
    .await
}

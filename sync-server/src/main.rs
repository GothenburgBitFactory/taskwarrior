use actix_web::{App, HttpServer};
use api::ServerState;
use server::{NullSyncServer, SyncServer};

mod api;
mod server;

// TODO: use hawk to sign requests

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let server_box: Box<dyn SyncServer> = Box::new(NullSyncServer::new());
    let server_state = ServerState::new(server_box);

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

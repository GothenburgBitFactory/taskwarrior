use actix_web::{get, web, App, HttpServer, Responder, Scope};
use api::{api_scope, ServerState};
use server::{InMemorySyncServer, SyncServer};

mod api;
mod server;

#[cfg(test)]
mod test;

// TODO: use hawk to sign requests

#[get("/")]
async fn index() -> impl Responder {
    // TODO: add version here
    "TaskChampion sync server"
}

/// Return a scope defining the URL rules for this server, with access to
/// the given ServerState.
pub(crate) fn app_scope(server_state: ServerState) -> Scope {
    web::scope("")
        .data(server_state)
        .service(index)
        .service(api_scope())
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    let server_box: Box<dyn SyncServer> = Box::new(InMemorySyncServer::new());
    let server_state = ServerState::new(server_box);

    HttpServer::new(move || App::new().service(app_scope(server_state.clone())))
        .bind("127.0.0.1:8080")?
        .run()
        .await
}

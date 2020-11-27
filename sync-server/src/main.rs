use crate::storage::{InMemoryStorage, Storage};
use actix_web::{get, web, App, HttpServer, Responder, Scope};
use api::{api_scope, ServerState};

mod api;
mod server;
mod storage;

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
    let server_box: Box<dyn Storage> = Box::new(InMemoryStorage::new());
    let server_state = ServerState::new(server_box);

    HttpServer::new(move || App::new().service(app_scope(server_state.clone())))
        .bind("127.0.0.1:8080")?
        .run()
        .await
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::api::ServerState;
    use crate::storage::{InMemoryStorage, Storage};
    use actix_web::{test, App};

    #[actix_rt::test]
    async fn test_index_get() {
        let server_box: Box<dyn Storage> = Box::new(InMemoryStorage::new());
        let server_state = ServerState::new(server_box);
        let mut app = test::init_service(App::new().service(app_scope(server_state))).await;

        let req = test::TestRequest::get().uri("/").to_request();
        let resp = test::call_service(&mut app, req).await;
        assert!(resp.status().is_success());
    }
}

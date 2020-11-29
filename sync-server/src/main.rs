use crate::storage::{KVStorage, Storage};
use actix_web::{get, web, App, HttpServer, Responder, Scope};
use api::{api_scope, ServerState};
use clap::Arg;
use failure::Fallible;

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
async fn main() -> Fallible<()> {
    env_logger::init();
    let matches = clap::App::new("taskchampion-sync-server")
        .version("0.1.0")
        .about("Server for TaskChampion")
        .arg(
            Arg::with_name("port")
                .short("p")
                .long("port")
                .value_name("PORT")
                .help("Port on which to serve")
                .default_value("8080")
                .takes_value(true)
                .required(true),
        )
        .arg(
            Arg::with_name("data-dir")
                .short("d")
                .long("data-dir")
                .value_name("DIR")
                .help("Directory in which to store data")
                .default_value("/var/lib/taskchampion-sync-server")
                .takes_value(true)
                .required(true),
        )
        .get_matches();

    let data_dir = matches.value_of("data-dir").unwrap();
    let port = matches.value_of("port").unwrap();

    let server_box: Box<dyn Storage> = Box::new(KVStorage::new(data_dir)?);
    let server_state = ServerState::new(server_box);

    log::warn!("Serving on port {}", port);
    HttpServer::new(move || App::new().service(app_scope(server_state.clone())))
        .bind(format!("0.0.0.0:{}", port))?
        .run()
        .await?;
    Ok(())
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

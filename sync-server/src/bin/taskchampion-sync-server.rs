#![deny(clippy::all)]

use actix_web::{middleware, middleware::Logger, App, HttpServer};
use clap::Arg;
use taskchampion_sync_server::storage::SqliteStorage;
use taskchampion_sync_server::Server;

// The `.wrap` method returns an opaque type, meaning that we can't easily return it from
// functions.  So, we must apply these default headers when the app is created, which occurs both
// in `main` and in the tests.  To check that those are both doing precisely the same thing, we use
// a macro.  This is ugly, and will go away when actix-web is no longer the framework in use.
macro_rules! cache_control_headers {
    ($wrapped:expr) => {
        $wrapped
            .wrap(middleware::DefaultHeaders::new().header("Cache-Control", "no-store, max-age=0"))
    };
}

#[actix_web::main]
async fn main() -> anyhow::Result<()> {
    env_logger::init();
    let matches = clap::App::new("taskchampion-sync-server")
        .version(env!("CARGO_PKG_VERSION"))
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

    let server = Server::new(Box::new(SqliteStorage::new(data_dir)?));

    log::warn!("Serving on port {}", port);
    HttpServer::new(move || {
        cache_control_headers!(App::new())
            .wrap(Logger::default())
            .service(server.service())
    })
    .bind(format!("0.0.0.0:{}", port))?
    .run()
    .await?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use actix_web::{test, App};
    use taskchampion_sync_server::storage::InMemoryStorage;

    #[actix_rt::test]
    async fn test_index_get() {
        let server = Server::new(Box::new(InMemoryStorage::new()));
        let app = cache_control_headers!(App::new()).service(server.service());
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

#![deny(clippy::all)]

use actix_web::{middleware::Logger, App, HttpServer};
use clap::{arg, builder::ValueParser, value_parser, Command};
use std::ffi::OsString;
use taskchampion_sync_server::storage::SqliteStorage;
use taskchampion_sync_server::{Server, ServerConfig};

#[actix_web::main]
async fn main() -> anyhow::Result<()> {
    env_logger::init();
    let defaults = ServerConfig::default();
    let default_snapshot_versions = defaults.snapshot_versions.to_string();
    let default_snapshot_days = defaults.snapshot_days.to_string();
    let matches = Command::new("taskchampion-sync-server")
        .version(env!("CARGO_PKG_VERSION"))
        .about("Server for TaskChampion")
        .arg(
            arg!(-p --port <PORT> "Port on which to serve")
                .help("Port on which to serve")
                .value_parser(value_parser!(usize))
                .default_value("8080"),
        )
        .arg(
            arg!(-d --"data-dir" <DIR> "Directory in which to store data")
                .value_parser(ValueParser::os_string())
                .default_value("/var/lib/taskchampion-sync-server"),
        )
        .arg(
            arg!(--"snapshot-versions" <NUM> "Target number of versions between snapshots")
                .value_parser(value_parser!(u32))
                .default_value(default_snapshot_versions),
        )
        .arg(
            arg!(--"snapshot-days" <NUM> "Target number of days between snapshots")
                .value_parser(value_parser!(i64))
                .default_value(default_snapshot_days),
        )
        .get_matches();

    let data_dir: &OsString = matches.get_one("data-dir").unwrap();
    let port: usize = *matches.get_one("port").unwrap();
    let snapshot_versions: u32 = *matches.get_one("snapshot-versions").unwrap();
    let snapshot_days: i64 = *matches.get_one("snapshot-days").unwrap();

    let config = ServerConfig::from_args(snapshot_days, snapshot_versions)?;
    let server = Server::new(config, Box::new(SqliteStorage::new(data_dir)?));

    log::warn!("Serving on port {}", port);
    HttpServer::new(move || {
        App::new()
            .wrap(Logger::default())
            .configure(|cfg| server.config(cfg))
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
        let server = Server::new(Default::default(), Box::new(InMemoryStorage::new()));
        let app = App::new().configure(|sc| server.config(sc));
        let mut app = test::init_service(app).await;

        let req = test::TestRequest::get().uri("/").to_request();
        let resp = test::call_service(&mut app, req).await;
        assert!(resp.status().is_success());
    }
}

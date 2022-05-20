use actix_web::{App, HttpServer};
use pretty_assertions::assert_eq;
use taskchampion::{Replica, ServerConfig, Status, StorageConfig, Uuid};
use taskchampion_sync_server::{
    storage::InMemoryStorage, Server, ServerConfig as SyncServerConfig,
};

const NUM_VERSIONS: u32 = 50;

#[actix_rt::test]
async fn sync_with_snapshots() -> anyhow::Result<()> {
    let _ = env_logger::builder()
        .is_test(true)
        .filter_level(log::LevelFilter::Trace)
        .try_init();

    let sync_server_config = SyncServerConfig {
        snapshot_days: 100,
        snapshot_versions: 3,
    };
    let server = Server::new(sync_server_config, Box::new(InMemoryStorage::new()));
    let httpserver =
        HttpServer::new(move || App::new().configure(|sc| server.config(sc))).bind("0.0.0.0:0")?;

    // bind was to :0, so the kernel will have selected an unused port
    let port = httpserver.addrs()[0].port();

    httpserver.run();

    let client_key = Uuid::new_v4();
    let encryption_secret = b"abc123".to_vec();
    let make_server = || {
        ServerConfig::Remote {
            origin: format!("http://127.0.0.1:{}", port),
            client_key,
            encryption_secret: encryption_secret.clone(),
        }
        .into_server()
    };

    // first we set up a single replica and sync it a lot of times, to establish a sync history.
    let mut rep1 = Replica::new(StorageConfig::InMemory.into_storage()?);
    let mut serv1 = make_server()?;

    let mut t1 = rep1.new_task(Status::Pending, "test 1".into())?;
    log::info!("Applying modifications on replica 1");
    for i in 0..=NUM_VERSIONS {
        let mut t1m = t1.into_mut(&mut rep1);
        t1m.start()?;
        t1m.stop()?;
        t1m.set_description(format!("revision {}", i))?;
        t1 = t1m.into_immut();

        rep1.sync(&mut serv1, false)?;
    }

    // now set up a second replica and sync it; it should catch up on that history, using a
    // snapshot.  Note that we can't verify that it used a snapshot, because the server currently
    // keeps all versions (so rep2 could sync from the beginning of the version history).  You can
    // manually verify that it is applying a snapshot by adding `assert!(false)` below and skimming
    // the logs.

    let mut rep2 = Replica::new(StorageConfig::InMemory.into_storage()?);
    let mut serv2 = make_server()?;

    log::info!("Syncing replica 2");
    rep2.sync(&mut serv2, false)?;

    // those tasks should exist on rep2 now
    let t12 = rep2
        .get_task(t1.get_uuid())?
        .expect("expected task 1 on rep2");

    assert_eq!(t12.get_description(), format!("revision {}", NUM_VERSIONS));
    assert_eq!(t12.is_active(), false);

    // sync that back to replica 1
    t12.into_mut(&mut rep2)
        .set_description("sync-back".to_owned())?;
    rep2.sync(&mut serv2, false)?;
    rep1.sync(&mut serv1, false)?;

    let t11 = rep1
        .get_task(t1.get_uuid())?
        .expect("expected task 1 on rep1");

    assert_eq!(t11.get_description(), "sync-back");

    // uncomment this to force a failure and see the logs
    // assert!(false);

    // note that we just drop the server here..
    Ok(())
}

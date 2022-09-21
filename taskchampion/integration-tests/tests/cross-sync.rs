use actix_web::{App, HttpServer};
use pretty_assertions::assert_eq;
use taskchampion::{Replica, ServerConfig, Status, StorageConfig, Uuid};
use taskchampion_sync_server::{storage::InMemoryStorage, Server};

#[actix_rt::test]
async fn cross_sync() -> anyhow::Result<()> {
    let _ = env_logger::builder()
        .is_test(true)
        .filter_level(log::LevelFilter::Trace)
        .try_init();

    let server = Server::new(Default::default(), Box::new(InMemoryStorage::new()));
    let httpserver =
        HttpServer::new(move || App::new().configure(|sc| server.config(sc))).bind("0.0.0.0:0")?;

    // bind was to :0, so the kernel will have selected an unused port
    let port = httpserver.addrs()[0].port();

    httpserver.run();

    // set up two replicas, and demonstrate replication between them
    let mut rep1 = Replica::new(StorageConfig::InMemory.into_storage()?);
    let mut rep2 = Replica::new(StorageConfig::InMemory.into_storage()?);

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

    let mut serv1 = make_server()?;
    let mut serv2 = make_server()?;

    // add some tasks on rep1
    let t1 = rep1.new_task(Status::Pending, "test 1".into())?;
    let t2 = rep1.new_task(Status::Pending, "test 2".into())?;

    // modify t1
    let mut t1 = t1.into_mut(&mut rep1);
    t1.start()?;
    let t1 = t1.into_immut();

    rep1.sync(&mut serv1, false)?;
    rep2.sync(&mut serv2, false)?;

    // those tasks should exist on rep2 now
    let t12 = rep2
        .get_task(t1.get_uuid())?
        .expect("expected task 1 on rep2");
    let t22 = rep2
        .get_task(t2.get_uuid())?
        .expect("expected task 2 on rep2");

    assert_eq!(t12.get_description(), "test 1");
    assert_eq!(t12.is_active(), true);
    assert_eq!(t22.get_description(), "test 2");
    assert_eq!(t22.is_active(), false);

    // make non-conflicting changes on the two replicas
    let mut t2 = t2.into_mut(&mut rep1);
    t2.set_status(Status::Completed)?;
    let t2 = t2.into_immut();

    let mut t12 = t12.into_mut(&mut rep2);
    t12.set_status(Status::Completed)?;

    // sync those changes back and forth
    rep1.sync(&mut serv1, false)?; // rep1 -> server
    rep2.sync(&mut serv2, false)?; // server -> rep2, rep2 -> server
    rep1.sync(&mut serv1, false)?; // server -> rep1

    let t1 = rep1
        .get_task(t1.get_uuid())?
        .expect("expected task 1 on rep1");
    assert_eq!(t1.get_status(), Status::Completed);

    let t22 = rep2
        .get_task(t2.get_uuid())?
        .expect("expected task 2 on rep2");
    assert_eq!(t22.get_status(), Status::Completed);

    // note that we just drop the server here..
    Ok(())
}

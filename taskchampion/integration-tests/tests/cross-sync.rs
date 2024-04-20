use pretty_assertions::assert_eq;
use taskchampion::{Replica, ServerConfig, Status, StorageConfig};
use tempfile::TempDir;

#[test]
fn cross_sync() -> anyhow::Result<()> {
    // set up two replicas, and demonstrate replication between them
    let mut rep1 = Replica::new(StorageConfig::InMemory.into_storage()?);
    let mut rep2 = Replica::new(StorageConfig::InMemory.into_storage()?);

    let tmp_dir = TempDir::new().expect("TempDir failed");
    let server_config = ServerConfig::Local {
        server_dir: tmp_dir.path().to_path_buf(),
    };
    let mut server = server_config.into_server()?;

    // add some tasks on rep1
    let t1 = rep1.new_task(Status::Pending, "test 1".into())?;
    let t2 = rep1.new_task(Status::Pending, "test 2".into())?;

    // modify t1
    let mut t1 = t1.into_mut(&mut rep1);
    t1.start()?;
    let t1 = t1.into_immut();

    rep1.sync(&mut server, false)?;
    rep2.sync(&mut server, false)?;

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
    rep1.sync(&mut server, false)?; // rep1 -> server
    rep2.sync(&mut server, false)?; // server -> rep2, rep2 -> server
    rep1.sync(&mut server, false)?; // server -> rep1

    let t1 = rep1
        .get_task(t1.get_uuid())?
        .expect("expected task 1 on rep1");
    assert_eq!(t1.get_status(), Status::Completed);

    let t22 = rep2
        .get_task(t2.get_uuid())?
        .expect("expected task 2 on rep2");
    assert_eq!(t22.get_status(), Status::Completed);

    Ok(())
}

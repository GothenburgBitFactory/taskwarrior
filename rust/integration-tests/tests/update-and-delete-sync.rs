use taskchampion::chrono::{TimeZone, Utc};
use taskchampion::{Replica, ServerConfig, Status, StorageConfig};
use tempfile::TempDir;

#[test]
fn update_and_delete_sync_delete_first() -> anyhow::Result<()> {
    update_and_delete_sync(true)
}

#[test]
fn update_and_delete_sync_update_first() -> anyhow::Result<()> {
    update_and_delete_sync(false)
}

/// Test what happens when an update is sync'd into a repo after a task is deleted.
/// If delete_first, then the deletion is sync'd to the server first; otherwise
/// the update is sync'd first.  Either way, the task is gone.
fn update_and_delete_sync(delete_first: bool) -> anyhow::Result<()> {
    // set up two replicas, and demonstrate replication between them
    let mut rep1 = Replica::new(StorageConfig::InMemory.into_storage()?);
    let mut rep2 = Replica::new(StorageConfig::InMemory.into_storage()?);

    let tmp_dir = TempDir::new().expect("TempDir failed");
    let mut server = ServerConfig::Local {
        server_dir: tmp_dir.path().to_path_buf(),
    }
    .into_server()?;

    // add a task on rep1, and sync it to rep2
    let t = rep1.new_task(Status::Pending, "test task".into())?;
    let u = t.get_uuid();

    rep1.sync(&mut server, false)?;
    rep2.sync(&mut server, false)?;

    // mark the task as deleted, long in the past, on rep2
    {
        let mut t = rep2.get_task(u)?.unwrap().into_mut(&mut rep2);
        t.delete()?;
        t.set_modified(Utc.ymd(1980, 1, 1).and_hms(0, 0, 0))?;
    }

    // sync it back to rep1
    rep2.sync(&mut server, false)?;
    rep1.sync(&mut server, false)?;

    // expire the task on rep1 and check that it is gone locally
    rep1.expire_tasks()?;
    assert!(rep1.get_task(u)?.is_none());

    // modify the task on rep2
    {
        let mut t = rep2.get_task(u)?.unwrap().into_mut(&mut rep2);
        t.set_description("modified".to_string())?;
    }

    // sync back and forth
    if delete_first {
        rep1.sync(&mut server, false)?;
    }
    rep2.sync(&mut server, false)?;
    rep1.sync(&mut server, false)?;
    if !delete_first {
        rep2.sync(&mut server, false)?;
    }

    // check that the task is gone on both replicas
    assert!(rep1.get_task(u)?.is_none());
    assert!(rep2.get_task(u)?.is_none());

    Ok(())
}

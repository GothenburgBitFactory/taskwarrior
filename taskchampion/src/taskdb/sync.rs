use super::{apply, snapshot};
use crate::server::{AddVersionResult, GetVersionResult, Server, SnapshotUrgency, SyncOp};
use crate::storage::StorageTxn;
use crate::Error;
use log::{info, trace, warn};
use serde::{Deserialize, Serialize};
use std::str;

#[derive(Serialize, Deserialize, Debug)]
struct Version {
    operations: Vec<SyncOp>,
}

/// Sync to the given server, pulling remote changes and pushing local changes.
pub(super) fn sync(
    server: &mut Box<dyn Server>,
    txn: &mut dyn StorageTxn,
    avoid_snapshots: bool,
) -> anyhow::Result<()> {
    // if this taskdb is entirely empty, then start by getting and applying a snapshot
    if txn.is_empty()? {
        trace!("storage is empty; attempting to apply a snapshot");
        if let Some((version, snap)) = server.get_snapshot()? {
            snapshot::apply_snapshot(txn, version, snap.as_ref())?;
            trace!("applied snapshot for version {}", version);
        }
    }

    // retry synchronizing until the server accepts our version (this allows for races between
    // replicas trying to sync to the same server).  If the server insists on the same base
    // version twice, then we have diverged.
    let mut requested_parent_version_id = None;
    loop {
        trace!("beginning sync outer loop");
        let mut base_version_id = txn.base_version()?;

        let mut local_ops: Vec<SyncOp> = txn
            .operations()?
            .drain(..)
            .filter_map(|op| op.into_sync())
            .collect();

        // first pull changes and "rebase" on top of them
        loop {
            trace!("beginning sync inner loop");
            if let GetVersionResult::Version {
                version_id,
                history_segment,
                ..
            } = server.get_child_version(base_version_id)?
            {
                let version_str = str::from_utf8(&history_segment).unwrap();
                let version: Version = serde_json::from_str(version_str).unwrap();

                // apply this verison and update base_version in storage
                info!("applying version {:?} from server", version_id);
                apply_version(txn, &mut local_ops, version)?;
                txn.set_base_version(version_id)?;
                base_version_id = version_id;
            } else {
                info!("no child versions of {:?}", base_version_id);
                // at the moment, no more child versions, so we can try adding our own
                break;
            }
        }

        if local_ops.is_empty() {
            info!("no changes to push to server");
            // nothing to sync back to the server..
            break;
        }

        trace!("sending {} operations to the server", local_ops.len());

        // now make a version of our local changes and push those
        let new_version = Version {
            operations: local_ops,
        };
        let history_segment = serde_json::to_string(&new_version).unwrap().into();
        info!("sending new version to server");
        let (res, snapshot_urgency) = server.add_version(base_version_id, history_segment)?;
        match res {
            AddVersionResult::Ok(new_version_id) => {
                info!("version {:?} received by server", new_version_id);
                txn.set_base_version(new_version_id)?;

                // make a snapshot if the server indicates it is urgent enough
                let base_urgency = if avoid_snapshots {
                    SnapshotUrgency::High
                } else {
                    SnapshotUrgency::Low
                };
                if snapshot_urgency >= base_urgency {
                    let snapshot = snapshot::make_snapshot(txn)?;
                    server.add_snapshot(new_version_id, snapshot)?;
                }

                break;
            }
            AddVersionResult::ExpectedParentVersion(parent_version_id) => {
                info!(
                    "new version rejected; must be based on {:?}",
                    parent_version_id
                );
                if let Some(requested) = requested_parent_version_id {
                    if parent_version_id == requested {
                        return Err(Error::OutOfSync.into());
                    }
                }
                requested_parent_version_id = Some(parent_version_id);
            }
        }
    }

    txn.set_operations(vec![])?;
    txn.commit()?;
    Ok(())
}

fn apply_version(
    txn: &mut dyn StorageTxn,
    local_ops: &mut Vec<SyncOp>,
    mut version: Version,
) -> anyhow::Result<()> {
    // The situation here is that the server has already applied all server operations, and we
    // have already applied all local operations, so states have diverged by several
    // operations.  We need to figure out what operations to apply locally and on the server in
    // order to return to the same state.
    //
    // Operational transforms provide this on an operation-by-operation basis.  To break this
    // down, we treat each server operation individually, in order.  For each such operation,
    // we start in this state:
    //
    //
    //      base state-*
    //                / \-server op
    //               *   *
    //     local    / \ /
    //     ops     *   *
    //            / \ / new
    //           *   * local
    //   local  / \ / ops
    //   state-*   *
    //      new-\ /
    // server op *-new local state
    //
    // This is slightly complicated by the fact that the transform function can return None,
    // indicating no operation is required.  If this happens for a local op, we can just omit
    // it.  If it happens for server op, then we must copy the remaining local ops.
    for server_op in version.operations.drain(..) {
        trace!(
            "rebasing local operations onto server operation {:?}",
            server_op
        );
        let mut new_local_ops = Vec::with_capacity(local_ops.len());
        let mut svr_op = Some(server_op);
        for local_op in local_ops.drain(..) {
            if let Some(o) = svr_op {
                let (new_server_op, new_local_op) = SyncOp::transform(o, local_op.clone());
                trace!("local operation {:?} -> {:?}", local_op, new_local_op);
                svr_op = new_server_op;
                if let Some(o) = new_local_op {
                    new_local_ops.push(o);
                }
            } else {
                trace!(
                    "local operation {:?} unchanged (server operation consumed)",
                    local_op
                );
                new_local_ops.push(local_op);
            }
        }
        if let Some(o) = svr_op {
            if let Err(e) = apply::apply_op(txn, &o) {
                warn!("Invalid operation when syncing: {} (ignored)", e);
            }
        }
        *local_ops = new_local_ops;
    }
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::server::{test::TestServer, SyncOp};
    use crate::storage::InMemoryStorage;
    use crate::taskdb::{snapshot::SnapshotTasks, TaskDb};
    use chrono::Utc;
    use pretty_assertions::assert_eq;
    use uuid::Uuid;

    fn newdb() -> TaskDb {
        TaskDb::new(Box::new(InMemoryStorage::new()))
    }

    #[test]
    fn test_sync() -> anyhow::Result<()> {
        let mut server: Box<dyn Server> = TestServer::new().server();

        let mut db1 = newdb();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();

        let mut db2 = newdb();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();

        // make some changes in parallel to db1 and db2..
        let uuid1 = Uuid::new_v4();
        db1.apply(SyncOp::Create { uuid: uuid1 }).unwrap();
        db1.apply(SyncOp::Update {
            uuid: uuid1,
            property: "title".into(),
            value: Some("my first task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        let uuid2 = Uuid::new_v4();
        db2.apply(SyncOp::Create { uuid: uuid2 }).unwrap();
        db2.apply(SyncOp::Update {
            uuid: uuid2,
            property: "title".into(),
            value: Some("my second task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        // now make updates to the same task on both sides
        db1.apply(SyncOp::Update {
            uuid: uuid2,
            property: "priority".into(),
            value: Some("H".into()),
            timestamp: Utc::now(),
        })
        .unwrap();
        db2.apply(SyncOp::Update {
            uuid: uuid2,
            property: "project".into(),
            value: Some("personal".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        Ok(())
    }

    #[test]
    fn test_sync_create_delete() -> anyhow::Result<()> {
        let mut server: Box<dyn Server> = TestServer::new().server();

        let mut db1 = newdb();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();

        let mut db2 = newdb();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();

        // create and update a task..
        let uuid = Uuid::new_v4();
        db1.apply(SyncOp::Create { uuid }).unwrap();
        db1.apply(SyncOp::Update {
            uuid,
            property: "title".into(),
            value: Some("my first task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        // delete and re-create the task on db1
        db1.apply(SyncOp::Delete { uuid }).unwrap();
        db1.apply(SyncOp::Create { uuid }).unwrap();
        db1.apply(SyncOp::Update {
            uuid,
            property: "title".into(),
            value: Some("my second task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and on db2, update a property of the task
        db2.apply(SyncOp::Update {
            uuid,
            property: "project".into(),
            value: Some("personal".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db2.storage.txn()?.as_mut(), false).unwrap();
        sync(&mut server, db1.storage.txn()?.as_mut(), false).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        Ok(())
    }

    #[test]
    fn test_sync_add_snapshot_start_with_snapshot() -> anyhow::Result<()> {
        let mut test_server = TestServer::new();

        let mut server: Box<dyn Server> = test_server.server();
        let mut db1 = newdb();

        let uuid = Uuid::new_v4();
        db1.apply(SyncOp::Create { uuid })?;
        db1.apply(SyncOp::Update {
            uuid,
            property: "title".into(),
            value: Some("my first task".into()),
            timestamp: Utc::now(),
        })?;

        test_server.set_snapshot_urgency(SnapshotUrgency::High);
        sync(&mut server, db1.storage.txn()?.as_mut(), false)?;

        // assert that a snapshot was added
        let base_version = db1.storage.txn()?.base_version()?;
        let (v, s) = test_server
            .snapshot()
            .ok_or_else(|| anyhow::anyhow!("no snapshot"))?;
        assert_eq!(v, base_version);

        let tasks = SnapshotTasks::decode(&s)?.into_inner();
        assert_eq!(tasks[0].0, uuid);

        // update the taskdb and sync again
        db1.apply(SyncOp::Update {
            uuid,
            property: "title".into(),
            value: Some("my first task, updated".into()),
            timestamp: Utc::now(),
        })?;
        sync(&mut server, db1.storage.txn()?.as_mut(), false)?;

        // delete the first version, so that db2 *must* initialize from
        // the snapshot
        test_server.delete_version(Uuid::nil());

        // sync to a new DB and check that we got the expected results
        let mut db2 = newdb();
        sync(&mut server, db2.storage.txn()?.as_mut(), false)?;

        let task = db2.get_task(uuid)?.unwrap();
        assert_eq!(task.get("title").unwrap(), "my first task, updated");

        Ok(())
    }

    #[test]
    fn test_sync_avoids_snapshot() -> anyhow::Result<()> {
        let test_server = TestServer::new();

        let mut server: Box<dyn Server> = test_server.server();
        let mut db1 = newdb();

        let uuid = Uuid::new_v4();
        db1.apply(SyncOp::Create { uuid }).unwrap();

        test_server.set_snapshot_urgency(SnapshotUrgency::Low);
        sync(&mut server, db1.storage.txn()?.as_mut(), true).unwrap();

        // assert that a snapshot was not added, because we indicated
        // we wanted to avoid snapshots and it was only low urgency
        assert_eq!(test_server.snapshot(), None);

        Ok(())
    }
}

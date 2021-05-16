use crate::errors::Error;
use crate::server::{AddVersionResult, GetVersionResult, Server};
use crate::storage::{Operation, Storage, StorageTxn, TaskMap};
use log::{info, trace, warn};
use serde::{Deserialize, Serialize};
use std::collections::HashSet;
use std::str;
use uuid::Uuid;

/// A TaskDb is the backend for a replica.  It manages the storage, operations, synchronization,
/// and so on, and all the invariants that come with it.  It leaves the meaning of particular task
/// properties to the replica and task implementations.
pub struct TaskDb {
    storage: Box<dyn Storage>,
}

#[derive(Serialize, Deserialize, Debug)]
struct Version {
    operations: Vec<Operation>,
}

impl TaskDb {
    /// Create a new TaskDb with the given backend storage
    pub fn new(storage: Box<dyn Storage>) -> TaskDb {
        TaskDb { storage }
    }

    #[cfg(test)]
    pub fn new_inmemory() -> TaskDb {
        TaskDb::new(Box::new(crate::storage::InMemoryStorage::new()))
    }

    /// Apply an operation to the TaskDb.  Aside from synchronization operations, this is the only way
    /// to modify the TaskDb.  In cases where an operation does not make sense, this function will do
    /// nothing and return an error (but leave the TaskDb in a consistent state).
    pub fn apply(&mut self, op: Operation) -> anyhow::Result<()> {
        // TODO: differentiate error types here?
        let mut txn = self.storage.txn()?;
        if let err @ Err(_) = TaskDb::apply_op(txn.as_mut(), &op) {
            return err;
        }
        txn.add_operation(op)?;
        txn.commit()?;
        Ok(())
    }

    fn apply_op(txn: &mut dyn StorageTxn, op: &Operation) -> anyhow::Result<()> {
        match op {
            Operation::Create { uuid } => {
                // insert if the task does not already exist
                if !txn.create_task(*uuid)? {
                    return Err(Error::DbError(format!("Task {} already exists", uuid)).into());
                }
            }
            Operation::Delete { ref uuid } => {
                if !txn.delete_task(*uuid)? {
                    return Err(Error::DbError(format!("Task {} does not exist", uuid)).into());
                }
            }
            Operation::Update {
                ref uuid,
                ref property,
                ref value,
                timestamp: _,
            } => {
                // update if this task exists, otherwise ignore
                if let Some(mut task) = txn.get_task(*uuid)? {
                    match value {
                        Some(ref val) => task.insert(property.to_string(), val.clone()),
                        None => task.remove(property),
                    };
                    txn.set_task(*uuid, task)?;
                } else {
                    return Err(Error::DbError(format!("Task {} does not exist", uuid)).into());
                }
            }
        }

        Ok(())
    }

    /// Get all tasks.
    pub fn all_tasks(&mut self) -> anyhow::Result<Vec<(Uuid, TaskMap)>> {
        let mut txn = self.storage.txn()?;
        txn.all_tasks()
    }

    /// Get the UUIDs of all tasks
    pub fn all_task_uuids(&mut self) -> anyhow::Result<Vec<Uuid>> {
        let mut txn = self.storage.txn()?;
        txn.all_task_uuids()
    }

    /// Get the working set
    pub fn working_set(&mut self) -> anyhow::Result<Vec<Option<Uuid>>> {
        let mut txn = self.storage.txn()?;
        txn.get_working_set()
    }

    /// Get a single task, by uuid.
    pub fn get_task(&mut self, uuid: Uuid) -> anyhow::Result<Option<TaskMap>> {
        let mut txn = self.storage.txn()?;
        txn.get_task(uuid)
    }

    /// Rebuild the working set using a function to identify tasks that should be in the set.  This
    /// renumbers the existing working-set tasks to eliminate gaps, and also adds any tasks that
    /// are not already in the working set but should be.  The rebuild occurs in a single
    /// trasnsaction against the storage backend.
    pub fn rebuild_working_set<F>(
        &mut self,
        in_working_set: F,
        renumber: bool,
    ) -> anyhow::Result<()>
    where
        F: Fn(&TaskMap) -> bool,
    {
        let mut txn = self.storage.txn()?;

        let mut new_ws = vec![];
        let mut seen = HashSet::new();

        // The goal here is for existing working-set items to be "compressed' down to index 1, so
        // we begin by scanning the current working set and inserting any tasks that should still
        // be in the set into new_ws, implicitly dropping any tasks that are no longer in the
        // working set.
        for elt in txn.get_working_set()? {
            if let Some(uuid) = elt {
                if let Some(task) = txn.get_task(uuid)? {
                    if in_working_set(&task) {
                        new_ws.push(Some(uuid));
                        seen.insert(uuid);
                        continue;
                    }
                }
            }

            // if we are not renumbering, then insert a blank working-set entry here
            if !renumber {
                new_ws.push(None);
            }
        }

        // if renumbering, clear the working set and re-add
        if renumber {
            txn.clear_working_set()?;
            for uuid in new_ws.drain(0..new_ws.len()).flatten() {
                txn.add_to_working_set(uuid)?;
            }
        } else {
            // ..otherwise, just clear the None items determined above from the working set
            for (i, elt) in new_ws.iter().enumerate() {
                if elt.is_none() {
                    txn.set_working_set_item(i, None)?;
                }
            }
        }

        // Now go hunting for tasks that should be in this list but are not, adding them at the
        // end of the list, whether renumbering or not
        for (uuid, task) in txn.all_tasks()? {
            if !seen.contains(&uuid) && in_working_set(&task) {
                txn.add_to_working_set(uuid)?;
            }
        }

        txn.commit()?;
        Ok(())
    }

    /// Add the given uuid to the working set and return its index; if it is already in the working
    /// set, its index is returned.  This does *not* renumber any existing tasks.
    pub fn add_to_working_set(&mut self, uuid: Uuid) -> anyhow::Result<usize> {
        let mut txn = self.storage.txn()?;
        // search for an existing entry for this task..
        for (i, elt) in txn.get_working_set()?.iter().enumerate() {
            if *elt == Some(uuid) {
                // (note that this drops the transaction with no changes made)
                return Ok(i);
            }
        }
        // and if not found, add one
        let i = txn.add_to_working_set(uuid)?;
        txn.commit()?;
        Ok(i)
    }

    /// Sync to the given server, pulling remote changes and pushing local changes.
    pub fn sync(&mut self, server: &mut Box<dyn Server>) -> anyhow::Result<()> {
        let mut txn = self.storage.txn()?;

        // retry synchronizing until the server accepts our version (this allows for races between
        // replicas trying to sync to the same server).  If the server insists on the same base
        // version twice, then we have diverged.
        let mut requested_parent_version_id = None;
        loop {
            trace!("beginning sync outer loop");
            let mut base_version_id = txn.base_version()?;

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
                    TaskDb::apply_version(txn.as_mut(), version)?;
                    txn.set_base_version(version_id)?;
                    base_version_id = version_id;
                } else {
                    info!("no child versions of {:?}", base_version_id);
                    // at the moment, no more child versions, so we can try adding our own
                    break;
                }
            }

            let operations: Vec<Operation> = txn.operations()?.to_vec();
            if operations.is_empty() {
                info!("no changes to push to server");
                // nothing to sync back to the server..
                break;
            }

            trace!("sending {} operations to the server", operations.len());

            // now make a version of our local changes and push those
            let new_version = Version { operations };
            let history_segment = serde_json::to_string(&new_version).unwrap().into();
            info!("sending new version to server");
            match server.add_version(base_version_id, history_segment)? {
                AddVersionResult::Ok(new_version_id) => {
                    info!("version {:?} received by server", new_version_id);
                    txn.set_base_version(new_version_id)?;
                    txn.set_operations(vec![])?;
                    break;
                }
                AddVersionResult::ExpectedParentVersion(parent_version_id) => {
                    info!(
                        "new version rejected; must be based on {:?}",
                        parent_version_id
                    );
                    if let Some(requested) = requested_parent_version_id {
                        if parent_version_id == requested {
                            anyhow::bail!("Server's task history has diverged from this replica");
                        }
                    }
                    requested_parent_version_id = Some(parent_version_id);
                }
            }
        }

        txn.commit()?;
        Ok(())
    }

    fn apply_version(txn: &mut dyn StorageTxn, mut version: Version) -> anyhow::Result<()> {
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
        let mut local_operations: Vec<Operation> = txn.operations()?;
        for server_op in version.operations.drain(..) {
            trace!(
                "rebasing local operations onto server operation {:?}",
                server_op
            );
            let mut new_local_ops = Vec::with_capacity(local_operations.len());
            let mut svr_op = Some(server_op);
            for local_op in local_operations.drain(..) {
                if let Some(o) = svr_op {
                    let (new_server_op, new_local_op) = Operation::transform(o, local_op.clone());
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
                if let Err(e) = TaskDb::apply_op(txn, &o) {
                    warn!("Invalid operation when syncing: {} (ignored)", e);
                }
            }
            local_operations = new_local_ops;
        }
        txn.set_operations(local_operations)?;
        Ok(())
    }

    // functions for supporting tests

    #[cfg(test)]
    pub(crate) fn sorted_tasks(&mut self) -> Vec<(Uuid, Vec<(String, String)>)> {
        let mut res: Vec<(Uuid, Vec<(String, String)>)> = self
            .all_tasks()
            .unwrap()
            .iter()
            .map(|(u, t)| {
                let mut t = t
                    .iter()
                    .map(|(p, v)| (p.clone(), v.clone()))
                    .collect::<Vec<(String, String)>>();
                t.sort();
                (u.clone(), t)
            })
            .collect();
        res.sort();
        res
    }

    #[cfg(test)]
    pub(crate) fn operations(&mut self) -> Vec<Operation> {
        let mut txn = self.storage.txn().unwrap();
        txn.operations()
            .unwrap()
            .iter()
            .map(|o| o.clone())
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::server::test::TestServer;
    use crate::storage::InMemoryStorage;
    use chrono::Utc;
    use proptest::prelude::*;
    use std::collections::HashMap;
    use uuid::Uuid;

    #[test]
    fn test_apply_create() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };
        db.apply(op.clone()).unwrap();

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![]),]);
        assert_eq!(db.operations(), vec![op]);
    }

    #[test]
    fn test_apply_create_exists() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };
        db.apply(op.clone()).unwrap();
        assert_eq!(
            db.apply(op.clone()).err().unwrap().to_string(),
            format!("Task Database Error: Task {} already exists", uuid)
        );

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![])]);
        assert_eq!(db.operations(), vec![op]);
    }

    #[test]
    fn test_apply_create_update() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        db.apply(op1.clone()).unwrap();
        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        db.apply(op2.clone()).unwrap();

        assert_eq!(
            db.sorted_tasks(),
            vec![(uuid, vec![("title".into(), "my task".into())])]
        );
        assert_eq!(db.operations(), vec![op1, op2]);
    }

    #[test]
    fn test_apply_create_update_delete_prop() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        db.apply(op1.clone()).unwrap();

        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        db.apply(op2.clone()).unwrap();

        let op3 = Operation::Update {
            uuid,
            property: String::from("priority"),
            value: Some("H".into()),
            timestamp: Utc::now(),
        };
        db.apply(op3.clone()).unwrap();

        let op4 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: None,
            timestamp: Utc::now(),
        };
        db.apply(op4.clone()).unwrap();

        let mut exp = HashMap::new();
        let mut task = HashMap::new();
        task.insert(String::from("priority"), String::from("H"));
        exp.insert(uuid, task);
        assert_eq!(
            db.sorted_tasks(),
            vec![(uuid, vec![("priority".into(), "H".into())])]
        );
        assert_eq!(db.operations(), vec![op1, op2, op3, op4]);
    }

    #[test]
    fn test_apply_update_does_not_exist() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        assert_eq!(
            db.apply(op).err().unwrap().to_string(),
            format!("Task Database Error: Task {} does not exist", uuid)
        );

        assert_eq!(db.sorted_tasks(), vec![]);
        assert_eq!(db.operations(), vec![]);
    }

    #[test]
    fn test_apply_create_delete() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        db.apply(op1.clone()).unwrap();

        let op2 = Operation::Delete { uuid };
        db.apply(op2.clone()).unwrap();

        assert_eq!(db.sorted_tasks(), vec![]);
        assert_eq!(db.operations(), vec![op1, op2]);
    }

    #[test]
    fn test_apply_delete_not_present() {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();

        let op1 = Operation::Delete { uuid };
        assert_eq!(
            db.apply(op1).err().unwrap().to_string(),
            format!("Task Database Error: Task {} does not exist", uuid)
        );

        assert_eq!(db.sorted_tasks(), vec![]);
        assert_eq!(db.operations(), vec![]);
    }

    #[test]
    fn rebuild_working_set_renumber() -> anyhow::Result<()> {
        rebuild_working_set(true)
    }

    #[test]
    fn rebuild_working_set_no_renumber() -> anyhow::Result<()> {
        rebuild_working_set(false)
    }

    fn rebuild_working_set(renumber: bool) -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let mut uuids = vec![];
        uuids.push(Uuid::new_v4());
        println!("uuids[0]: {:?} - pending, not in working set", uuids[0]);
        uuids.push(Uuid::new_v4());
        println!("uuids[1]: {:?} - pending, in working set", uuids[1]);
        uuids.push(Uuid::new_v4());
        println!("uuids[2]: {:?} - not pending, not in working set", uuids[2]);
        uuids.push(Uuid::new_v4());
        println!("uuids[3]: {:?} - not pending, in working set", uuids[3]);
        uuids.push(Uuid::new_v4());
        println!("uuids[4]: {:?} - pending, in working set", uuids[4]);

        // add everything to the TaskDb
        for uuid in &uuids {
            db.apply(Operation::Create { uuid: *uuid })?;
        }
        for i in &[0usize, 1, 4] {
            db.apply(Operation::Update {
                uuid: uuids[*i].clone(),
                property: String::from("status"),
                value: Some("pending".into()),
                timestamp: Utc::now(),
            })?;
        }

        // set the existing working_set as we want it
        {
            let mut txn = db.storage.txn()?;
            txn.clear_working_set()?;

            for i in &[1usize, 3, 4] {
                txn.add_to_working_set(uuids[*i])?;
            }

            txn.commit()?;
        }

        assert_eq!(
            db.working_set()?,
            vec![
                None,
                Some(uuids[1].clone()),
                Some(uuids[3].clone()),
                Some(uuids[4].clone())
            ]
        );

        db.rebuild_working_set(
            |t| {
                if let Some(status) = t.get("status") {
                    status == "pending"
                } else {
                    false
                }
            },
            renumber,
        )?;

        let exp = if renumber {
            // uuids[1] and uuids[4] are already in the working set, so are compressed
            // to the top, and then uuids[0] is added.
            vec![
                None,
                Some(uuids[1].clone()),
                Some(uuids[4].clone()),
                Some(uuids[0].clone()),
            ]
        } else {
            // uuids[1] and uuids[4] are already in the working set, at indexes 1 and 3,
            // and then uuids[0] is added.
            vec![
                None,
                Some(uuids[1].clone()),
                None,
                Some(uuids[4].clone()),
                Some(uuids[0].clone()),
            ]
        };

        assert_eq!(db.working_set()?, exp);

        Ok(())
    }

    fn newdb() -> TaskDb {
        TaskDb::new(Box::new(InMemoryStorage::new()))
    }

    #[test]
    fn test_sync() {
        let mut server: Box<dyn Server> = Box::new(TestServer::new());

        let mut db1 = newdb();
        db1.sync(&mut server).unwrap();

        let mut db2 = newdb();
        db2.sync(&mut server).unwrap();

        // make some changes in parallel to db1 and db2..
        let uuid1 = Uuid::new_v4();
        db1.apply(Operation::Create { uuid: uuid1 }).unwrap();
        db1.apply(Operation::Update {
            uuid: uuid1,
            property: "title".into(),
            value: Some("my first task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        let uuid2 = Uuid::new_v4();
        db2.apply(Operation::Create { uuid: uuid2 }).unwrap();
        db2.apply(Operation::Update {
            uuid: uuid2,
            property: "title".into(),
            value: Some("my second task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        db1.sync(&mut server).unwrap();
        db2.sync(&mut server).unwrap();
        db1.sync(&mut server).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        // now make updates to the same task on both sides
        db1.apply(Operation::Update {
            uuid: uuid2,
            property: "priority".into(),
            value: Some("H".into()),
            timestamp: Utc::now(),
        })
        .unwrap();
        db2.apply(Operation::Update {
            uuid: uuid2,
            property: "project".into(),
            value: Some("personal".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        db1.sync(&mut server).unwrap();
        db2.sync(&mut server).unwrap();
        db1.sync(&mut server).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
    }

    #[test]
    fn test_sync_create_delete() {
        let mut server: Box<dyn Server> = Box::new(TestServer::new());

        let mut db1 = newdb();
        db1.sync(&mut server).unwrap();

        let mut db2 = newdb();
        db2.sync(&mut server).unwrap();

        // create and update a task..
        let uuid = Uuid::new_v4();
        db1.apply(Operation::Create { uuid }).unwrap();
        db1.apply(Operation::Update {
            uuid: uuid,
            property: "title".into(),
            value: Some("my first task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and synchronize those around
        db1.sync(&mut server).unwrap();
        db2.sync(&mut server).unwrap();
        db1.sync(&mut server).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

        // delete and re-create the task on db1
        db1.apply(Operation::Delete { uuid }).unwrap();
        db1.apply(Operation::Create { uuid }).unwrap();
        db1.apply(Operation::Update {
            uuid: uuid,
            property: "title".into(),
            value: Some("my second task".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        // and on db2, update a property of the task
        db2.apply(Operation::Update {
            uuid: uuid,
            property: "project".into(),
            value: Some("personal".into()),
            timestamp: Utc::now(),
        })
        .unwrap();

        db1.sync(&mut server).unwrap();
        db2.sync(&mut server).unwrap();
        db1.sync(&mut server).unwrap();
        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
    }

    #[derive(Debug)]
    enum Action {
        Op(Operation),
        Sync,
    }

    fn action_sequence_strategy() -> impl Strategy<Value = Vec<(Action, u8)>> {
        // Create, Update, Delete, or Sync on client 1, 2, .., followed by a round of syncs
        "([CUDS][123])*S1S2S3S1S2".prop_map(|seq| {
            let uuid = Uuid::parse_str("83a2f9ef-f455-4195-b92e-a54c161eebfc").unwrap();
            seq.as_bytes()
                .chunks(2)
                .map(|action_on| {
                    let action = match action_on[0] {
                        b'C' => Action::Op(Operation::Create { uuid }),
                        b'U' => Action::Op(Operation::Update {
                            uuid,
                            property: "title".into(),
                            value: Some("foo".into()),
                            timestamp: Utc::now(),
                        }),
                        b'D' => Action::Op(Operation::Delete { uuid }),
                        b'S' => Action::Sync,
                        _ => unreachable!(),
                    };
                    let acton = action_on[1] - b'1';
                    (action, acton)
                })
                .collect::<Vec<(Action, u8)>>()
        })
    }

    proptest! {
        #[test]
        // check that various sequences of operations on mulitple db's do not get the db's into an
        // incompatible state.  The main concern here is that there might be a sequence of create
        // and delete operations that results in a task existing in one TaskDb but not existing in
        // another.  So, the generated sequences focus on a single task UUID.
        fn transform_sequences_of_operations(action_sequence in action_sequence_strategy()) {
        let mut server: Box<dyn Server> = Box::new(TestServer::new());
            let mut dbs = [newdb(), newdb(), newdb()];

            for (action, db) in action_sequence {
                println!("{:?} on db {}", action, db);

                let db = &mut dbs[db as usize];
                match action {
                    Action::Op(op) => {
                        if let Err(e) = db.apply(op) {
                            println!("  {:?} (ignored)", e);
                        }
                    },
                    Action::Sync => db.sync(&mut server).unwrap(),
                }
            }

            assert_eq!(dbs[0].sorted_tasks(), dbs[0].sorted_tasks());
            assert_eq!(dbs[1].sorted_tasks(), dbs[2].sorted_tasks());
        }
    }
}

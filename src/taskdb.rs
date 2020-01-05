use crate::errors::Error;
use crate::operation::Operation;
use crate::server::{Server, VersionAdd};
use crate::taskstorage::{InMemoryStorage, TaskMap};
use failure::Fallible;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::str;
use uuid::Uuid;

#[derive(Debug, Clone)]
pub struct DB {
    storage: InMemoryStorage,
}

#[derive(Serialize, Deserialize, Debug)]
struct Version {
    version: u64,
    operations: Vec<Operation>,
}

impl DB {
    /// Create a new, empty database
    pub fn new() -> DB {
        DB {
            storage: InMemoryStorage::new(),
        }
    }

    /// Apply an operation to the DB.  Aside from synchronization operations, this is the only way
    /// to modify the DB.  In cases where an operation does not make sense, this function will do
    /// nothing and return an error (but leave the DB in a consistent state).
    pub fn apply(&mut self, op: Operation) -> Fallible<()> {
        if let err @ Err(_) = self.apply_op(&op) {
            return err;
        }
        self.storage.add_operation(op);
        Ok(())
    }

    fn apply_op(&mut self, op: &Operation) -> Fallible<()> {
        match op {
            &Operation::Create { uuid } => {
                // insert if the task does not already exist
                if !self.storage.create_task(uuid, HashMap::new()) {
                    return Err(Error::DBError(format!("Task {} already exists", uuid)).into());
                }
            }
            &Operation::Delete { ref uuid } => {
                if !self.storage.delete_task(uuid) {
                    return Err(Error::DBError(format!("Task {} does not exist", uuid)).into());
                }
            }
            &Operation::Update {
                ref uuid,
                ref property,
                ref value,
                timestamp: _,
            } => {
                // update if this task exists, otherwise ignore
                if let Some(task) = self.storage.get_task(uuid) {
                    let mut task = task.clone();
                    match value {
                        Some(ref val) => task.insert(property.to_string(), val.clone()),
                        None => task.remove(property),
                    };
                    self.storage.set_task(uuid.clone(), task);
                } else {
                    return Err(Error::DBError(format!("Task {} does not exist", uuid)).into());
                }
            }
        }

        Ok(())
    }

    /// Get all tasks.  This is not a terribly efficient operation.
    pub fn all_tasks<'a>(&'a self) -> impl Iterator<Item = (Uuid, TaskMap)> + 'a {
        self.all_task_uuids()
            .map(move |u| (u, self.get_task(&u).unwrap()))
    }

    /// Get the UUIDs of all tasks
    pub fn all_task_uuids<'a>(&'a self) -> impl Iterator<Item = Uuid> + 'a {
        self.storage.get_task_uuids()
    }

    /// Get a single task, by uuid.
    pub fn get_task(&self, uuid: &Uuid) -> Option<TaskMap> {
        self.storage.get_task(uuid)
    }

    /// Sync to the given server, pulling remote changes and pushing local changes.
    pub fn sync(&mut self, username: &str, server: &mut Server) {
        // retry synchronizing until the server accepts our version (this allows for races between
        // replicas trying to sync to the same server)
        loop {
            // first pull changes and "rebase" on top of them
            let new_versions = server.get_versions(username, self.storage.base_version());
            for version_blob in new_versions {
                let version_str = str::from_utf8(&version_blob).unwrap();
                let version: Version = serde_json::from_str(version_str).unwrap();
                assert_eq!(version.version, self.storage.base_version() + 1);
                println!("applying version {:?} from server", version.version);

                self.apply_version(version);
            }

            let operations: Vec<Operation> = self.storage.operations().map(|o| o.clone()).collect();
            if operations.len() == 0 {
                // nothing to sync back to the server..
                break;
            }

            // now make a version of our local changes and push those
            let new_version = Version {
                version: self.storage.base_version() + 1,
                operations: operations,
            };
            let new_version_str = serde_json::to_string(&new_version).unwrap();
            println!("sending version {:?} to server", new_version.version);
            if let VersionAdd::Ok =
                server.add_version(username, new_version.version, new_version_str.into())
            {
                self.storage.local_operations_synced(new_version.version);
                break;
            }
        }
    }

    fn apply_version(&mut self, mut version: Version) {
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
        let mut local_operations: Vec<Operation> =
            self.storage.operations().map(|o| o.clone()).collect();
        for server_op in version.operations.drain(..) {
            let mut new_local_ops = Vec::with_capacity(local_operations.len());
            let mut svr_op = Some(server_op);
            for local_op in local_operations.drain(..) {
                if let Some(o) = svr_op {
                    let (new_server_op, new_local_op) = Operation::transform(o, local_op);
                    svr_op = new_server_op;
                    if let Some(o) = new_local_op {
                        new_local_ops.push(o);
                    }
                } else {
                    new_local_ops.push(local_op);
                }
            }
            if let Some(o) = svr_op {
                if let Err(e) = self.apply_op(&o) {
                    println!("Invalid operation when syncing: {} (ignored)", e);
                }
            }
            local_operations = new_local_ops;
        }
        self.storage
            .update_version(version.version, local_operations);
    }

    // functions for supporting tests

    pub fn sorted_tasks(&self) -> Vec<(Uuid, Vec<(String, String)>)> {
        let mut res: Vec<(Uuid, Vec<(String, String)>)> = self
            .all_tasks()
            .map(|(u, t)| {
                let mut t = t
                    .iter()
                    .map(|(p, v)| (p.clone(), v.clone()))
                    .collect::<Vec<(String, String)>>();
                t.sort();
                (u, t)
            })
            .collect();
        res.sort();
        res
    }

    pub fn operations(&self) -> Vec<Operation> {
        self.storage.operations().map(|o| o.clone()).collect()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use chrono::Utc;
    use uuid::Uuid;

    #[test]
    fn test_apply_create() {
        let mut db = DB::new();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };
        db.apply(op.clone()).unwrap();

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![]),]);
        assert_eq!(db.operations(), vec![op]);
    }

    #[test]
    fn test_apply_create_exists() {
        let mut db = DB::new();
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
        let mut db = DB::new();
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
        let mut db = DB::new();
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
        let mut db = DB::new();
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
        let mut db = DB::new();
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
        let mut db = DB::new();
        let uuid = Uuid::new_v4();

        let op1 = Operation::Delete { uuid };
        assert_eq!(
            db.apply(op1).err().unwrap().to_string(),
            format!("Task Database Error: Task {} does not exist", uuid)
        );

        assert_eq!(db.sorted_tasks(), vec![]);
        assert_eq!(db.operations(), vec![]);
    }
}

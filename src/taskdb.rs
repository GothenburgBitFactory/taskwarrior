use crate::operation::Operation;
use crate::server::{Server, VersionAdd};
use serde::{Deserialize, Serialize};
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use std::str;
use uuid::Uuid;

type TaskMap = HashMap<String, String>;

#[derive(PartialEq, Debug, Clone)]
pub struct DB {
    // The current state, with all pending operations applied
    tasks: HashMap<Uuid, TaskMap>,

    // The version at which `operations` begins
    base_version: u64,

    // Operations applied since `base_version`, in order.
    //
    // INVARIANT: Given a snapshot at `base_version`, applying these operations produces `tasks`.
    operations: Vec<Operation>,
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
            tasks: HashMap::new(),
            base_version: 0,
            operations: vec![],
        }
    }

    /// Apply an operation to the DB.  Aside from synchronization operations, this
    /// is the only way to modify the DB.  In cases where an operation does not
    /// make sense, this function will ignore the operation.
    pub fn apply(&mut self, op: Operation) {
        match op {
            Operation::Create { uuid } => {
                // insert if the task does not already exist
                if let ent @ Entry::Vacant(_) = self.tasks.entry(uuid) {
                    ent.or_insert(HashMap::new());
                }
            }
            Operation::Update {
                ref uuid,
                ref property,
                ref value,
                timestamp: _,
            } => {
                // update if this task exists, otherwise ignore
                if let Some(task) = self.tasks.get_mut(uuid) {
                    DB::apply_update(task, property, value);
                }
            }
        };
        self.operations.push(op);
    }

    fn apply_update(task: &mut TaskMap, property: &str, value: &Option<String>) {
        match value {
            Some(ref val) => task.insert(property.to_string(), val.clone()),
            None => task.remove(property),
        };
    }

    /// Get a read-only reference to the underlying set of tasks.
    ///
    /// This API is temporary, but provides query access to the DB.
    pub fn tasks(&self) -> &HashMap<Uuid, TaskMap> {
        &self.tasks
    }

    /// Sync to the given server, pulling remote changes and pushing local changes.
    pub fn sync(&mut self, username: &str, server: &mut Server) {
        loop {
            // first pull changes and "rebase" on top of them
            let new_versions = server.get_versions(username, self.base_version);
            for version_blob in new_versions {
                let version_str = str::from_utf8(&version_blob).unwrap();
                let version: Version = serde_json::from_str(version_str).unwrap();
                assert_eq!(version.version, self.base_version + 1);
                println!("applying version {:?} from server", version.version);

                self.apply_version(version);
            }

            if self.operations.len() == 0 {
                break;
            }

            // now make a version of our local changes and push those
            let new_version = Version {
                version: self.base_version + 1,
                operations: self.operations.clone(),
            };
            let new_version_str = serde_json::to_string(&new_version).unwrap();
            println!("sending version {:?} to server", new_version.version);
            if let VersionAdd::Ok =
                server.add_version(username, new_version.version, new_version_str.into())
            {
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
        for server_op in version.operations.drain(..) {
            let mut new_local_ops = Vec::with_capacity(self.operations.len());
            let mut svr_op = Some(server_op);
            for local_op in self.operations.drain(..) {
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
                self.apply(o);
            }
            self.operations = new_local_ops;
        }
        self.base_version = version.version;
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
        db.apply(op.clone());

        let mut exp = HashMap::new();
        exp.insert(uuid, HashMap::new());
        assert_eq!(db.tasks(), &exp);
        assert_eq!(db.operations, vec![op]);
    }

    #[test]
    fn test_apply_create_exists() {
        let mut db = DB::new();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };
        db.apply(op.clone());
        db.apply(op.clone());

        let mut exp = HashMap::new();
        exp.insert(uuid, HashMap::new());
        assert_eq!(db.tasks(), &exp);
        assert_eq!(db.operations, vec![op.clone(), op]);
    }

    #[test]
    fn test_apply_create_update() {
        let mut db = DB::new();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        db.apply(op1.clone());
        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        db.apply(op2.clone());

        let mut exp = HashMap::new();
        let mut task = HashMap::new();
        task.insert(String::from("title"), String::from("my task"));
        exp.insert(uuid, task);
        assert_eq!(db.tasks(), &exp);
        assert_eq!(db.operations, vec![op1, op2]);
    }

    #[test]
    fn test_apply_create_update_delete_prop() {
        let mut db = DB::new();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        db.apply(op1.clone());

        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        db.apply(op2.clone());

        let op3 = Operation::Update {
            uuid,
            property: String::from("priority"),
            value: Some("H".into()),
            timestamp: Utc::now(),
        };
        db.apply(op3.clone());

        let op4 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: None,
            timestamp: Utc::now(),
        };
        db.apply(op4.clone());

        let mut exp = HashMap::new();
        let mut task = HashMap::new();
        task.insert(String::from("priority"), String::from("H"));
        exp.insert(uuid, task);
        assert_eq!(db.tasks(), &exp);
        assert_eq!(db.operations, vec![op1, op2, op3, op4]);
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
        db.apply(op.clone());

        assert_eq!(db.tasks(), &HashMap::new());
        assert_eq!(db.operations, vec![op]);
    }
}

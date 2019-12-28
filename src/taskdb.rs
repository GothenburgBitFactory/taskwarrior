use crate::operation::Operation;
use serde_json::Value;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use uuid::Uuid;

#[derive(PartialEq, Debug, Clone)]
pub struct DB {
    // The current state, with all pending operations applied
    tasks: HashMap<Uuid, HashMap<String, Value>>,

    // The version at which `operations` begins
    base_version: u64,

    // Operations applied since `base_version`, in order.
    //
    // INVARIANT: Given a snapshot at `base_version`, applying these operations produces `tasks`.
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
                    task.insert(property.clone(), value.clone());
                }
            }
        };
        self.operations.push(op);
    }

    /// Get a read-only reference to the underlying set of tasks.
    ///
    /// This API is temporary, but provides query access to the DB.
    pub fn tasks(&self) -> &HashMap<Uuid, HashMap<String, Value>> {
        &self.tasks
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
            value: Value::from("\"my task\""),
            timestamp: Utc::now(),
        };
        db.apply(op2.clone());

        let mut exp = HashMap::new();
        let mut task = HashMap::new();
        task.insert(String::from("title"), Value::from("\"my task\""));
        exp.insert(uuid, task);
        assert_eq!(db.tasks(), &exp);
        assert_eq!(db.operations, vec![op1, op2]);
    }

    #[test]
    fn test_apply_update_does_not_exist() {
        let mut db = DB::new();
        let uuid = Uuid::new_v4();
        let op = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Value::from("\"my task\""),
            timestamp: Utc::now(),
        };
        db.apply(op.clone());

        assert_eq!(db.tasks(), &HashMap::new());
        assert_eq!(db.operations, vec![op]);
    }
}

use crate::operation::Operation;
use crate::taskdb::DB;
use chrono::Utc;
use failure::Fallible;
use std::collections::HashMap;
use uuid::Uuid;

/// A replica represents an instance of a user's task data.
pub struct Replica {
    taskdb: Box<DB>,
}

impl Replica {
    pub fn new(taskdb: Box<DB>) -> Replica {
        return Replica { taskdb };
    }

    /// Create a new task.  The task must not already exist.
    pub fn create_task(&mut self, uuid: Uuid) -> Fallible<()> {
        self.taskdb.apply(Operation::Create { uuid })
    }

    /// Delete a task.  The task must exist.
    pub fn delete_task(&mut self, uuid: Uuid) -> Fallible<()> {
        self.taskdb.apply(Operation::Delete { uuid })
    }

    /// Update an existing task.  If the value is Some, the property is added or updated.  If the
    /// value is None, the property is deleted.  It is not an error to delete a nonexistent
    /// property.
    pub fn update_task<S1, S2>(
        &mut self,
        uuid: Uuid,
        property: S1,
        value: Option<S2>,
    ) -> Fallible<()>
    where
        S1: Into<String>,
        S2: Into<String>,
    {
        self.taskdb.apply(Operation::Update {
            uuid,
            property: property.into(),
            value: value.map(|v| v.into()),
            timestamp: Utc::now(),
        })
    }

    /// Get all tasks as an iterator of (&Uuid, &HashMap)
    pub fn all_tasks(&self) -> impl Iterator<Item = (&Uuid, &HashMap<String, String>)> {
        self.taskdb.tasks().iter()
    }

    /// Get an existing task by its UUID
    pub fn get_task(&self, uuid: &Uuid) -> Option<&HashMap<String, String>> {
        self.taskdb.tasks().get(&uuid)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::taskdb::DB;
    use uuid::Uuid;

    #[test]
    fn create() {
        let mut rep = Replica::new(DB::new().into());
        let uuid = Uuid::new_v4();

        rep.create_task(uuid.clone()).unwrap();
        assert_eq!(rep.get_task(&uuid), Some(&HashMap::new()));
    }

    #[test]
    fn delete() {
        let mut rep = Replica::new(DB::new().into());
        let uuid = Uuid::new_v4();

        rep.create_task(uuid.clone()).unwrap();
        rep.delete_task(uuid.clone()).unwrap();
        assert_eq!(rep.get_task(&uuid), None);
    }

    #[test]
    fn update() {
        let mut rep = Replica::new(DB::new().into());
        let uuid = Uuid::new_v4();

        rep.create_task(uuid.clone()).unwrap();
        rep.update_task(uuid.clone(), "title", Some("snarsblat"))
            .unwrap();
        let mut task = HashMap::new();
        task.insert("title".into(), "snarsblat".into());
        assert_eq!(rep.get_task(&uuid), Some(&task));
    }

    #[test]
    fn get_does_not_exist() {
        let rep = Replica::new(DB::new().into());
        let uuid = Uuid::new_v4();
        assert_eq!(rep.get_task(&uuid), None);
    }
}

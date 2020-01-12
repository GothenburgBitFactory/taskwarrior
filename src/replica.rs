use crate::operation::Operation;
use crate::task::{Priority, Status, Task, TaskBuilder};
use crate::taskdb::DB;
use crate::taskstorage::TaskMap;
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

    /// Update an existing task.  If the value is Some, the property is added or updated.  If the
    /// value is None, the property is deleted.  It is not an error to delete a nonexistent
    /// property.
    fn update_task<S1, S2>(&mut self, uuid: Uuid, property: S1, value: Option<S2>) -> Fallible<()>
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

    /// Get all tasks represented as a map keyed by UUID
    pub fn all_tasks<'a>(&'a mut self) -> Fallible<HashMap<Uuid, Task>> {
        Ok(self
            .taskdb
            .all_tasks()?
            .iter()
            .map(|(k, v)| (k.clone(), v.into()))
            .collect())
    }

    /// Get the UUIDs of all tasks
    pub fn all_task_uuids<'a>(&'a mut self) -> Fallible<Vec<Uuid>> {
        self.taskdb.all_task_uuids()
    }

    /// Get an existing task by its UUID
    pub fn get_task(&mut self, uuid: &Uuid) -> Fallible<Option<Task>> {
        Ok(self.taskdb.get_task(&uuid)?.map(|t| (&t).into()))
    }

    /// Create a new task.  The task must not already exist.
    pub fn new_task(
        &mut self,
        uuid: Uuid,
        status: Status,
        description: String,
    ) -> Fallible<TaskMut> {
        // TODO: check that it doesn't exist
        self.taskdb
            .apply(Operation::Create { uuid: uuid.clone() })?;
        self.update_task(uuid.clone(), "status", Some(String::from(status.as_ref())))?;
        self.update_task(uuid.clone(), "description", Some(description))?;
        let now = format!("{}", Utc::now().timestamp());
        self.update_task(uuid.clone(), "entry", Some(now.clone()))?;
        self.update_task(uuid.clone(), "modified", Some(now))?;
        Ok(TaskMut::new(self, uuid))
    }

    /// Delete a task.  The task must exist.
    pub fn delete_task(&mut self, uuid: Uuid) -> Fallible<()> {
        // TODO: must verify task does exist
        self.taskdb.apply(Operation::Delete { uuid })
    }

    /// Get an existing task by its UUID, suitable for modification
    pub fn get_task_mut<'a>(&'a mut self, uuid: &Uuid) -> Fallible<Option<TaskMut<'a>>> {
        // the call to get_task is to ensure the task exists locally
        Ok(self
            .taskdb
            .get_task(&uuid)?
            .map(move |_| TaskMut::new(self, uuid.clone())))
    }
}

impl From<&TaskMap> for Task {
    fn from(taskmap: &TaskMap) -> Task {
        let mut bldr = TaskBuilder::new();
        for (k, v) in taskmap.iter() {
            bldr = bldr.set(k, v.into());
        }
        bldr.finish()
    }
}

/// TaskMut allows changes to a task.  It is intended for short-term use, such as changing a few
/// properties, and should not be held for long periods of wall-clock time.
pub struct TaskMut<'a> {
    replica: &'a mut Replica,
    uuid: Uuid,
    // if true, then this TaskMut has already updated the `modified` property and need not do so
    // again.
    updated_modified: bool,
}

impl<'a> TaskMut<'a> {
    fn new(replica: &'a mut Replica, uuid: Uuid) -> TaskMut {
        TaskMut {
            replica,
            uuid,
            updated_modified: false,
        }
    }

    fn lastmod(&mut self) -> Fallible<()> {
        if !self.updated_modified {
            let now = format!("{}", Utc::now().timestamp());
            self.replica
                .update_task(self.uuid.clone(), "modified", Some(now))?;
            self.updated_modified = true;
        }
        Ok(())
    }

    /// Set the task's status
    pub fn status(&mut self, status: Status) -> Fallible<()> {
        self.lastmod()?;
        self.replica.update_task(
            self.uuid.clone(),
            "status",
            Some(String::from(status.as_ref())),
        )
    }

    // TODO: description
    // TODO: start
    // TODO: end
    // TODO: due
    // TODO: until
    // TODO: wait
    // TODO: scheduled
    // TODO: recur
    // TODO: mask
    // TODO: imask
    // TODO: parent

    /// Set the task's project
    pub fn project(&mut self, project: String) -> Fallible<()> {
        self.lastmod()?;
        self.replica
            .update_task(self.uuid.clone(), "project", Some(project))
    }

    /// Set the task's priority
    pub fn priority(&mut self, priority: Priority) -> Fallible<()> {
        self.lastmod()?;
        self.replica.update_task(
            self.uuid.clone(),
            "priority",
            Some(String::from(priority.as_ref())),
        )
    }

    // TODO: depends
    // TODO: tags
    // TODO: annotations
    // TODO: udas
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::taskdb::DB;
    use uuid::Uuid;

    #[test]
    fn new_task_and_modify() {
        let mut rep = Replica::new(DB::new_inmemory().into());
        let uuid = Uuid::new_v4();

        let mut tm = rep
            .new_task(uuid.clone(), Status::Pending, "a task".into())
            .unwrap();
        tm.priority(Priority::L).unwrap();

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.description, String::from("a task"));
        assert_eq!(t.status, Status::Pending);
        assert_eq!(t.priority, Some(Priority::L));
    }

    #[test]
    fn delete_task() {
        let mut rep = Replica::new(DB::new_inmemory().into());
        let uuid = Uuid::new_v4();

        rep.new_task(uuid.clone(), Status::Pending, "a task".into())
            .unwrap();

        rep.delete_task(uuid.clone()).unwrap();
        assert_eq!(rep.get_task(&uuid).unwrap(), None);
    }

    #[test]
    fn get_and_modify() {
        let mut rep = Replica::new(DB::new_inmemory().into());
        let uuid = Uuid::new_v4();

        rep.new_task(uuid.clone(), Status::Pending, "another task".into())
            .unwrap();

        let mut tm = rep.get_task_mut(&uuid).unwrap().unwrap();
        tm.priority(Priority::L).unwrap();
        tm.project("work".into()).unwrap();

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.description, String::from("another task"));
        assert_eq!(t.project, Some("work".into()));
    }

    #[test]
    fn get_does_not_exist() {
        let mut rep = Replica::new(DB::new_inmemory().into());
        let uuid = Uuid::new_v4();
        assert_eq!(rep.get_task(&uuid).unwrap(), None);
    }
}

use crate::errors::Error;
use crate::operation::Operation;
use crate::task::{Priority, Status, Task, TaskBuilder};
use crate::taskdb::DB;
use crate::taskstorage::TaskMap;
use chrono::{DateTime, Utc};
use failure::Fallible;
use itertools::join;
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

    /// Get the "working set" for this replica -- the set of pending tasks, as indexed by small
    /// integers
    pub fn working_set(&mut self) -> Fallible<Vec<Option<(Uuid, Task)>>> {
        let working_set = self.taskdb.working_set()?;
        let mut res = Vec::with_capacity(working_set.len());
        for i in 0..working_set.len() {
            res.push(match working_set[i] {
                Some(u) => match self.taskdb.get_task(&u)? {
                    Some(task) => Some((u, (&task).into())),
                    None => None,
                },
                None => None,
            })
        }
        Ok(res)
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
        // check that it doesn't exist; this is a convenience check, as the task
        // may already exist when this Create operation is finally sync'd with
        // operations from other replicas
        if self.taskdb.get_task(&uuid)?.is_some() {
            return Err(Error::DBError(format!("Task {} already exists", uuid)).into());
        }
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
        // check that it already exists; this is a convenience check, as the task may already exist
        // when this Create operation is finally sync'd with operations from other replicas
        if self.taskdb.get_task(&uuid)?.is_none() {
            return Err(Error::DBError(format!("Task {} does not exist", uuid)).into());
        }
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

    /// Perform "garbage collection" on this replica.  In particular, this renumbers the working
    /// set.
    pub fn gc(&mut self) -> Fallible<()> {
        self.taskdb.rebuild_working_set()?;
        Ok(())
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

// TODO: move this struct to crate::task, with a trait for update_task, since it is the reverse
// of TaskBuilder::set
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

    fn set_string(&mut self, property: &str, value: Option<String>) -> Fallible<()> {
        self.lastmod()?;
        self.replica.update_task(self.uuid.clone(), property, value)
    }

    fn set_timestamp(&mut self, property: &str, value: Option<DateTime<Utc>>) -> Fallible<()> {
        self.lastmod()?;
        self.replica.update_task(
            self.uuid.clone(),
            property,
            value.map(|v| format!("{}", v.timestamp())),
        )
    }

    /// Set the task's status
    pub fn status(&mut self, status: Status) -> Fallible<()> {
        self.set_string("status", Some(String::from(status.as_ref())))
    }

    /// Set the task's description
    pub fn description(&mut self, description: String) -> Fallible<()> {
        self.set_string("description", Some(description))
    }

    /// Set the task's start time
    pub fn start(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("start", time)
    }

    /// Set the task's end time
    pub fn end(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("end", time)
    }

    /// Set the task's due time
    pub fn due(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("due", time)
    }

    /// Set the task's until time
    pub fn until(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("until", time)
    }

    /// Set the task's wait time
    pub fn wait(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("wait", time)
    }

    /// Set the task's scheduled time
    pub fn scheduled(&mut self, time: Option<DateTime<Utc>>) -> Fallible<()> {
        self.set_timestamp("scheduled", time)
    }

    /// Set the task's recur value
    pub fn recur(&mut self, recur: Option<String>) -> Fallible<()> {
        self.set_string("recur", recur)
    }

    /// Set the task's mask value
    pub fn mask(&mut self, mask: Option<String>) -> Fallible<()> {
        self.set_string("mask", mask)
    }

    /// Set the task's imask value
    pub fn imask(&mut self, imask: Option<u64>) -> Fallible<()> {
        self.set_string("imask", imask.map(|v| format!("{}", v)))
    }

    /// Set the task's parent task
    pub fn parent(&mut self, parent: Option<Uuid>) -> Fallible<()> {
        self.set_string("parent", parent.map(|v| format!("{}", v)))
    }

    /// Set the task's project
    pub fn project(&mut self, project: Option<String>) -> Fallible<()> {
        self.set_string("project", project)
    }

    /// Set the task's priority
    pub fn priority(&mut self, priority: Option<Priority>) -> Fallible<()> {
        self.set_string("priority", priority.map(|v| String::from(v.as_ref())))
    }

    /// Set the task's depends; note that this completely replaces the list of tasks on which this
    /// one depends.
    pub fn depends(&mut self, depends: Vec<Uuid>) -> Fallible<()> {
        self.set_string(
            "depends",
            if depends.len() > 0 {
                Some(join(depends.iter(), ","))
            } else {
                None
            },
        )
    }

    /// Set the task's tags; note that this completely replaces the list of tags
    pub fn tags(&mut self, tags: Vec<Uuid>) -> Fallible<()> {
        self.set_string("tags", Some(join(tags.iter(), ",")))
    }

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
        tm.priority(Some(Priority::L)).unwrap();

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

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.description, String::from("another task"));

        let mut tm = rep.get_task_mut(&uuid).unwrap().unwrap();
        tm.status(Status::Completed).unwrap();
        tm.description("another task, updated".into()).unwrap();
        tm.priority(Some(Priority::L)).unwrap();
        tm.project(Some("work".into())).unwrap();

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.status, Status::Completed);
        assert_eq!(t.description, String::from("another task, updated"));
        assert_eq!(t.project, Some("work".into()));
    }

    #[test]
    fn get_does_not_exist() {
        let mut rep = Replica::new(DB::new_inmemory().into());
        let uuid = Uuid::new_v4();
        assert_eq!(rep.get_task(&uuid).unwrap(), None);
    }
}

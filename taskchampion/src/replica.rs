use crate::config::ReplicaConfig;
use crate::errors::Error;
use crate::server::Server;
use crate::task::{Status, Task};
use crate::taskdb::TaskDB;
use crate::taskstorage::{KVStorage, Operation, TaskMap, TaskStorage};
use chrono::Utc;
use failure::Fallible;
use log::trace;
use std::collections::HashMap;
use uuid::Uuid;

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
///
/// ## Tasks
///
/// Tasks are uniquely identified by UUIDs.
/// Most task modifications are performed via the [`crate::Task`] and [`crate::TaskMut`] types.
///
/// ## Working Set
///
/// A replica maintains a "working set" of tasks that are of current concern to the user,
/// specifically pending tasks.  These are indexed with small, easy-to-type integers.  Newly
/// pending tasks are automatically added to the working set, and the working set is "renumbered"
/// during the garbage-collection process.
pub struct Replica {
    taskdb: TaskDB,
}

impl Replica {
    pub fn new(storage: Box<dyn TaskStorage>) -> Replica {
        Replica {
            taskdb: TaskDB::new(storage),
        }
    }

    /// Construct a new replica from a configuration object.  This is the common way
    /// to create a new object.
    pub fn from_config(config: ReplicaConfig) -> Fallible<Replica> {
        let storage = Box::new(KVStorage::new(config.taskdb_dir)?);
        Ok(Replica::new(storage))
    }

    #[cfg(test)]
    pub fn new_inmemory() -> Replica {
        Replica::new(Box::new(crate::taskstorage::InMemoryStorage::new()))
    }

    /// Update an existing task.  If the value is Some, the property is added or updated.  If the
    /// value is None, the property is deleted.  It is not an error to delete a nonexistent
    /// property.
    pub(crate) fn update_task<S1, S2>(
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

    /// Add the given uuid to the working set, returning its index.
    pub(crate) fn add_to_working_set(&mut self, uuid: &Uuid) -> Fallible<usize> {
        self.taskdb.add_to_working_set(uuid)
    }

    /// Get all tasks represented as a map keyed by UUID
    pub fn all_tasks(&mut self) -> Fallible<HashMap<Uuid, Task>> {
        let mut res = HashMap::new();
        for (uuid, tm) in self.taskdb.all_tasks()?.drain(..) {
            res.insert(uuid, Task::new(uuid, tm));
        }
        Ok(res)
    }

    /// Get the UUIDs of all tasks
    pub fn all_task_uuids(&mut self) -> Fallible<Vec<Uuid>> {
        self.taskdb.all_task_uuids()
    }

    /// Get the "working set" for this replica -- the set of pending tasks, as indexed by small
    /// integers
    pub fn working_set(&mut self) -> Fallible<Vec<Option<Task>>> {
        let working_set = self.taskdb.working_set()?;
        let mut res = Vec::with_capacity(working_set.len());
        for item in working_set.iter() {
            res.push(match item {
                Some(u) => match self.taskdb.get_task(&u)? {
                    Some(tm) => Some(Task::new(*u, tm)),
                    None => None,
                },
                None => None,
            })
        }
        Ok(res)
    }

    /// Get an existing task by its UUID
    pub fn get_task(&mut self, uuid: &Uuid) -> Fallible<Option<Task>> {
        Ok(self
            .taskdb
            .get_task(uuid)?
            .map(move |tm| Task::new(*uuid, tm)))
    }

    /// Get an existing task by its working set index
    pub fn get_working_set_task(&mut self, i: usize) -> Fallible<Option<Task>> {
        let working_set = self.taskdb.working_set()?;
        if (i as usize) < working_set.len() {
            if let Some(uuid) = working_set[i as usize] {
                return Ok(self
                    .taskdb
                    .get_task(&uuid)?
                    .map(move |tm| Task::new(uuid, tm)));
            }
        }
        Ok(None)
    }

    /// Get the working set index for the given task uuid
    pub fn get_working_set_index(&mut self, uuid: &Uuid) -> Fallible<Option<usize>> {
        let working_set = self.taskdb.working_set()?;
        for (i, u) in working_set.iter().enumerate() {
            if let Some(ref u) = u {
                if u == uuid {
                    return Ok(Some(i));
                }
            }
        }
        Ok(None)
    }

    /// Create a new task.  The task must not already exist.
    pub fn new_task(&mut self, status: Status, description: String) -> Fallible<Task> {
        let uuid = Uuid::new_v4();
        self.taskdb.apply(Operation::Create { uuid })?;
        trace!("task {} created", uuid);
        let mut task = Task::new(uuid, TaskMap::new()).into_mut(self);
        task.set_description(description)?;
        task.set_status(status)?;
        Ok(task.into_immut())
    }

    /// Delete a task.  The task must exist.  Note that this is different from setting status to
    /// Deleted; this is the final purge of the task.  This is not a public method as deletion
    /// should only occur through expiration.
    #[allow(dead_code)]
    fn delete_task(&mut self, uuid: &Uuid) -> Fallible<()> {
        // check that it already exists; this is a convenience check, as the task may already exist
        // when this Create operation is finally sync'd with operations from other replicas
        if self.taskdb.get_task(&uuid)?.is_none() {
            return Err(Error::DBError(format!("Task {} does not exist", uuid)).into());
        }
        self.taskdb.apply(Operation::Delete { uuid: *uuid })?;
        trace!("task {} deleted", uuid);
        Ok(())
    }

    /// Synchronize this replica against the given server.
    pub fn sync(&mut self, server: &mut Box<dyn Server>) -> Fallible<()> {
        self.taskdb.sync(server)
    }

    /// Perform "garbage collection" on this replica.  In particular, this renumbers the working
    /// set to contain only pending tasks.
    pub fn gc(&mut self) -> Fallible<()> {
        let pending = String::from(Status::Pending.to_taskmap());
        self.taskdb
            .rebuild_working_set(|t| t.get("status") == Some(&pending))?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::task::Status;
    use uuid::Uuid;

    #[test]
    fn new_task() {
        let mut rep = Replica::new_inmemory();

        let t = rep.new_task(Status::Pending, "a task".into()).unwrap();
        assert_eq!(t.get_description(), String::from("a task"));
        assert_eq!(t.get_status(), Status::Pending);
        assert!(t.get_modified().is_some());
    }

    #[test]
    fn modify_task() {
        let mut rep = Replica::new_inmemory();

        let t = rep.new_task(Status::Pending, "a task".into()).unwrap();

        let mut t = t.into_mut(&mut rep);
        t.set_description(String::from("past tense")).unwrap();
        t.set_status(Status::Completed).unwrap();
        // check that values have changed on the TaskMut
        assert_eq!(t.get_description(), "past tense");
        assert_eq!(t.get_status(), Status::Completed);

        // check that values have changed after into_immut
        let t = t.into_immut();
        assert_eq!(t.get_description(), "past tense");
        assert_eq!(t.get_status(), Status::Completed);

        // check tha values have changed in storage, too
        let t = rep.get_task(&t.get_uuid()).unwrap().unwrap();
        assert_eq!(t.get_description(), "past tense");
        assert_eq!(t.get_status(), Status::Completed);
    }

    #[test]
    fn delete_task() {
        let mut rep = Replica::new_inmemory();

        let t = rep.new_task(Status::Pending, "a task".into()).unwrap();
        let uuid = t.get_uuid();

        rep.delete_task(uuid).unwrap();
        assert_eq!(rep.get_task(&uuid).unwrap(), None);
    }

    #[test]
    fn get_and_modify() {
        let mut rep = Replica::new_inmemory();

        let t = rep
            .new_task(Status::Pending, "another task".into())
            .unwrap();
        let uuid = t.get_uuid();

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.get_description(), String::from("another task"));

        let mut t = t.into_mut(&mut rep);
        t.set_status(Status::Deleted).unwrap();
        t.set_description("gone".into()).unwrap();

        let t = rep.get_task(&uuid).unwrap().unwrap();
        assert_eq!(t.get_status(), Status::Deleted);
        assert_eq!(t.get_description(), "gone");

        rep.gc().unwrap();

        assert!(rep.get_working_set_index(t.get_uuid()).unwrap().is_none());
    }

    #[test]
    fn new_pending_adds_to_working_set() {
        let mut rep = Replica::new_inmemory();

        let t = rep
            .new_task(Status::Pending, "to-be-pending".into())
            .unwrap();
        let uuid = t.get_uuid();

        let t = rep.get_working_set_task(1).unwrap().unwrap();
        assert_eq!(t.get_status(), Status::Pending);
        assert_eq!(t.get_description(), "to-be-pending");

        let ws = rep.working_set().unwrap();
        assert_eq!(ws.len(), 2);
        assert!(ws[0].is_none());
        assert_eq!(ws[1].as_ref().unwrap().get_uuid(), uuid);

        assert_eq!(rep.get_working_set_index(t.get_uuid()).unwrap().unwrap(), 1);
    }

    #[test]
    fn get_does_not_exist() {
        let mut rep = Replica::new_inmemory();
        let uuid = Uuid::new_v4();
        assert_eq!(rep.get_task(&uuid).unwrap(), None);
    }
}

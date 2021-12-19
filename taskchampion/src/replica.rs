use crate::server::{Server, SyncOp};
use crate::storage::{Storage, TaskMap};
use crate::task::{Status, Task};
use crate::taskdb::TaskDb;
use crate::workingset::WorkingSet;
use anyhow::Context;
use chrono::Utc;
use log::trace;
use std::collections::HashMap;
use uuid::Uuid;

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
///
/// ## Tasks
///
/// Tasks are uniquely identified by UUIDs.
/// Most task modifications are performed via the [`Task`](crate::Task) and
/// [`TaskMut`](crate::TaskMut) types.  Use of two types for tasks allows easy
/// read-only manipulation of lots of tasks, with exclusive access required only
/// for modifications.
///
/// ## Working Set
///
/// A replica maintains a "working set" of tasks that are of current concern to the user,
/// specifically pending tasks.  These are indexed with small, easy-to-type integers.  Newly
/// pending tasks are automatically added to the working set, and the working set is "renumbered"
/// during the garbage-collection process.
pub struct Replica {
    taskdb: TaskDb,
}

impl Replica {
    pub fn new(storage: Box<dyn Storage>) -> Replica {
        Replica {
            taskdb: TaskDb::new(storage),
        }
    }

    #[cfg(test)]
    pub fn new_inmemory() -> Replica {
        Replica::new(Box::new(crate::storage::InMemoryStorage::new()))
    }

    /// Update an existing task.  If the value is Some, the property is added or updated.  If the
    /// value is None, the property is deleted.  It is not an error to delete a nonexistent
    /// property.
    pub(crate) fn update_task<S1, S2>(
        &mut self,
        uuid: Uuid,
        property: S1,
        value: Option<S2>,
    ) -> anyhow::Result<TaskMap>
    where
        S1: Into<String>,
        S2: Into<String>,
    {
        self.taskdb.apply(SyncOp::Update {
            uuid,
            property: property.into(),
            value: value.map(|v| v.into()),
            timestamp: Utc::now(),
        })
    }

    /// Add the given uuid to the working set, returning its index.
    pub(crate) fn add_to_working_set(&mut self, uuid: Uuid) -> anyhow::Result<usize> {
        self.taskdb.add_to_working_set(uuid)
    }

    /// Get all tasks represented as a map keyed by UUID
    pub fn all_tasks(&mut self) -> anyhow::Result<HashMap<Uuid, Task>> {
        let mut res = HashMap::new();
        for (uuid, tm) in self.taskdb.all_tasks()?.drain(..) {
            res.insert(uuid, Task::new(uuid, tm));
        }
        Ok(res)
    }

    /// Get the UUIDs of all tasks
    pub fn all_task_uuids(&mut self) -> anyhow::Result<Vec<Uuid>> {
        self.taskdb.all_task_uuids()
    }

    /// Get the "working set" for this replica.  This is a snapshot of the current state,
    /// and it is up to the caller to decide how long to store this value.
    pub fn working_set(&mut self) -> anyhow::Result<WorkingSet> {
        Ok(WorkingSet::new(self.taskdb.working_set()?))
    }

    /// Get an existing task by its UUID
    pub fn get_task(&mut self, uuid: Uuid) -> anyhow::Result<Option<Task>> {
        Ok(self
            .taskdb
            .get_task(uuid)?
            .map(move |tm| Task::new(uuid, tm)))
    }

    /// Create a new task.  The task must not already exist.
    pub fn new_task(&mut self, status: Status, description: String) -> anyhow::Result<Task> {
        let uuid = Uuid::new_v4();
        let taskmap = self.taskdb.apply(SyncOp::Create { uuid })?;
        trace!("task {} created", uuid);
        let mut task = Task::new(uuid, taskmap).into_mut(self);
        task.set_description(description)?;
        task.set_status(status)?;
        Ok(task.into_immut())
    }

    /// Delete a task.  The task must exist.  Note that this is different from setting status to
    /// Deleted; this is the final purge of the task.  This is not a public method as deletion
    /// should only occur through expiration.
    #[allow(dead_code)]
    fn delete_task(&mut self, uuid: Uuid) -> anyhow::Result<()> {
        self.taskdb.apply(SyncOp::Delete { uuid })?;
        trace!("task {} deleted", uuid);
        Ok(())
    }

    /// Synchronize this replica against the given server.  The working set is rebuilt after
    /// this occurs, but without renumbering, so any newly-pending tasks should appear in
    /// the working set.
    ///
    /// If `avoid_snapshots` is true, the sync operations produces a snapshot only when the server
    /// indicate it is urgent (snapshot urgency "high").  This allows time for other replicas to
    /// create a snapshot before this one does.
    ///
    /// Set this to true on systems more constrained in CPU, memory, or bandwidth than a typical desktop
    /// system
    pub fn sync(
        &mut self,
        server: &mut Box<dyn Server>,
        avoid_snapshots: bool,
    ) -> anyhow::Result<()> {
        self.taskdb
            .sync(server, avoid_snapshots)
            .context("Failed to synchronize with server")?;
        self.rebuild_working_set(false)
            .context("Failed to rebuild working set after sync")?;
        Ok(())
    }

    /// Rebuild this replica's working set, based on whether tasks are pending or not.  If
    /// `renumber` is true, then existing tasks may be moved to new working-set indices; in any
    /// case, on completion all pending tasks are in the working set and all non- pending tasks are
    /// not.
    pub fn rebuild_working_set(&mut self, renumber: bool) -> anyhow::Result<()> {
        let pending = String::from(Status::Pending.to_taskmap());
        self.taskdb
            .rebuild_working_set(|t| t.get("status") == Some(&pending), renumber)?;
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::task::Status;
    use pretty_assertions::assert_eq;
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
        let t = rep.get_task(t.get_uuid()).unwrap().unwrap();
        assert_eq!(t.get_description(), "past tense");
        assert_eq!(t.get_status(), Status::Completed);
    }

    #[test]
    fn delete_task() {
        let mut rep = Replica::new_inmemory();

        let t = rep.new_task(Status::Pending, "a task".into()).unwrap();
        let uuid = t.get_uuid();

        rep.delete_task(uuid).unwrap();
        assert_eq!(rep.get_task(uuid).unwrap(), None);
    }

    #[test]
    fn get_and_modify() {
        let mut rep = Replica::new_inmemory();

        let t = rep
            .new_task(Status::Pending, "another task".into())
            .unwrap();
        let uuid = t.get_uuid();

        let t = rep.get_task(uuid).unwrap().unwrap();
        assert_eq!(t.get_description(), String::from("another task"));

        let mut t = t.into_mut(&mut rep);
        t.set_status(Status::Deleted).unwrap();
        t.set_description("gone".into()).unwrap();

        let t = rep.get_task(uuid).unwrap().unwrap();
        assert_eq!(t.get_status(), Status::Deleted);
        assert_eq!(t.get_description(), "gone");

        rep.rebuild_working_set(true).unwrap();

        let ws = rep.working_set().unwrap();
        assert!(ws.by_uuid(t.get_uuid()).is_none());
    }

    #[test]
    fn new_pending_adds_to_working_set() {
        let mut rep = Replica::new_inmemory();

        let t = rep
            .new_task(Status::Pending, "to-be-pending".into())
            .unwrap();
        let uuid = t.get_uuid();

        let ws = rep.working_set().unwrap();
        assert_eq!(ws.len(), 1); // only one non-none value
        assert!(ws.by_index(0).is_none());
        assert_eq!(ws.by_index(1), Some(uuid));

        let ws = rep.working_set().unwrap();
        assert_eq!(ws.by_uuid(t.get_uuid()), Some(1));
    }

    #[test]
    fn get_does_not_exist() {
        let mut rep = Replica::new_inmemory();
        let uuid = Uuid::new_v4();
        assert_eq!(rep.get_task(uuid).unwrap(), None);
    }
}

use crate::depmap::DependencyMap;
use crate::server::{Server, SyncOp};
use crate::storage::{Storage, TaskMap};
use crate::task::{Status, Task};
use crate::taskdb::TaskDb;
use crate::workingset::WorkingSet;
use anyhow::Context;
use chrono::{Duration, Utc};
use log::trace;
use std::collections::HashMap;
use std::rc::Rc;
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

    /// If true, this replica has already added an undo point.
    added_undo_point: bool,

    /// The dependency map for this replica, if it has been calculated.
    depmap: Option<Rc<DependencyMap>>,
}

impl Replica {
    pub fn new(storage: Box<dyn Storage>) -> Replica {
        Replica {
            taskdb: TaskDb::new(storage),
            added_undo_point: false,
            depmap: None,
        }
    }

    #[cfg(test)]
    pub fn new_inmemory() -> Replica {
        Replica::new(Box::new(crate::storage::InMemoryStorage::new()))
    }

    /// Update an existing task.  If the value is Some, the property is added or updated.  If the
    /// value is None, the property is deleted.  It is not an error to delete a nonexistent
    /// property.
    ///
    /// This is a low-level method, and requires knowledge of the Task data model.  Prefer to
    /// use the [`TaskMut`] methods to modify tasks, where possible.
    pub fn update_task<S1, S2>(
        &mut self,
        uuid: Uuid,
        property: S1,
        value: Option<S2>,
    ) -> anyhow::Result<TaskMap>
    where
        S1: Into<String>,
        S2: Into<String>,
    {
        self.add_undo_point(false)?;
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
        let depmap = self.dependency_map(false)?;
        let mut res = HashMap::new();
        for (uuid, tm) in self.taskdb.all_tasks()?.drain(..) {
            res.insert(uuid, Task::new(uuid, tm, depmap.clone()));
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

    /// Get the dependency map for all pending tasks.
    ///
    /// The data in this map is cached when it is first requested and may not contain modifications
    /// made locally in this Replica instance.  The result is reference-counted and may
    /// outlive the Replica.
    ///
    /// If `force` is true, then the result is re-calculated from the current state of the replica,
    /// although previously-returned dependency maps are not updated.
    pub fn dependency_map(&mut self, force: bool) -> anyhow::Result<Rc<DependencyMap>> {
        if force || self.depmap.is_none() {
            let mut dm = DependencyMap::new();
            let ws = self.working_set()?;
            for i in 1..=ws.largest_index() {
                if let Some(u) = ws.by_index(i) {
                    // note: we can't use self.get_task here, as that depends on a
                    // DependencyMap
                    if let Some(taskmap) = self.taskdb.get_task(u)? {
                        for p in taskmap.keys() {
                            if let Some(dep_str) = p.strip_prefix("dep_") {
                                if let Ok(dep) = Uuid::parse_str(dep_str) {
                                    dm.add_dependency(u, dep);
                                }
                            }
                        }
                    }
                }
            }
            self.depmap = Some(Rc::new(dm));
        }

        // at this point self.depmap is guaranteed to be Some(_)
        Ok(self.depmap.as_ref().unwrap().clone())
    }

    /// Get an existing task by its UUID
    pub fn get_task(&mut self, uuid: Uuid) -> anyhow::Result<Option<Task>> {
        let depmap = self.dependency_map(false)?;
        Ok(self
            .taskdb
            .get_task(uuid)?
            .map(move |tm| Task::new(uuid, tm, depmap)))
    }

    /// Create a new task.
    pub fn new_task(&mut self, status: Status, description: String) -> anyhow::Result<Task> {
        let uuid = Uuid::new_v4();
        self.add_undo_point(false)?;
        let taskmap = self.taskdb.apply(SyncOp::Create { uuid })?;
        let depmap = self.dependency_map(false)?;
        let mut task = Task::new(uuid, taskmap, depmap).into_mut(self);
        task.set_description(description)?;
        task.set_status(status)?;
        task.set_entry(Some(Utc::now()))?;
        trace!("task {} created", uuid);
        Ok(task.into_immut())
    }

    /// Create a new, empty task with the given UUID.  This is useful for importing tasks, but
    /// otherwise should be avoided in favor of `new_task`.  If the task already exists, this
    /// does nothing and returns the existing task.
    pub fn import_task_with_uuid(&mut self, uuid: Uuid) -> anyhow::Result<Task> {
        self.add_undo_point(false)?;
        let taskmap = self.taskdb.apply(SyncOp::Create { uuid })?;
        let depmap = self.dependency_map(false)?;
        Ok(Task::new(uuid, taskmap, depmap))
    }

    /// Delete a task.  The task must exist.  Note that this is different from setting status to
    /// Deleted; this is the final purge of the task.  This is not a public method as deletion
    /// should only occur through expiration.
    fn delete_task(&mut self, uuid: Uuid) -> anyhow::Result<()> {
        self.add_undo_point(false)?;
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

    /// Undo local operations until the most recent UndoPoint, returning false if there are no
    /// local operations to undo.
    pub fn undo(&mut self) -> anyhow::Result<bool> {
        self.taskdb.undo()
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

    /// Expire old, deleted tasks.
    ///
    /// Expiration entails removal of tasks from the replica. Any modifications that occur after
    /// the deletion (such as operations synchronized from other replicas) will do nothing.
    ///
    /// Tasks are eligible for expiration when they have status Deleted and have not been modified
    /// for 180 days (about six months). Note that completed tasks are not eligible.
    pub fn expire_tasks(&mut self) -> anyhow::Result<()> {
        let six_mos_ago = Utc::now() - Duration::days(180);
        self.all_tasks()?
            .iter()
            .filter(|(_, t)| t.get_status() == Status::Deleted)
            .filter(|(_, t)| {
                if let Some(m) = t.get_modified() {
                    m < six_mos_ago
                } else {
                    false
                }
            })
            .try_for_each(|(u, _)| self.delete_task(*u))?;
        Ok(())
    }

    /// Add an UndoPoint, if one has not already been added by this Replica.  This occurs
    /// automatically when a change is made.  The `force` flag allows forcing a new UndoPoint
    /// even if one has already been created by this Replica, and may be useful when a Replica
    /// instance is held for a long time and used to apply more than one user-visible change.
    pub fn add_undo_point(&mut self, force: bool) -> anyhow::Result<()> {
        if force || !self.added_undo_point {
            self.taskdb.add_undo_point()?;
            self.added_undo_point = true;
        }
        Ok(())
    }

    /// Get the number of operations local to this replica and not yet synchronized to the server.
    pub fn num_local_operations(&mut self) -> anyhow::Result<usize> {
        self.taskdb.num_operations()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::storage::ReplicaOp;
    use crate::task::Status;
    use chrono::TimeZone;
    use pretty_assertions::assert_eq;
    use std::collections::HashSet;
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

        // check that values have changed in storage, too
        let t = rep.get_task(t.get_uuid()).unwrap().unwrap();
        assert_eq!(t.get_description(), "past tense");
        assert_eq!(t.get_status(), Status::Completed);

        // and check for the corresponding operations, cleaning out the timestamps
        // and modified properties as these are based on the current time
        let now = Utc::now();
        let clean_op = |op: ReplicaOp| {
            if let ReplicaOp::Update {
                uuid,
                property,
                mut old_value,
                mut value,
                ..
            } = op
            {
                // rewrite automatically-created dates to "just-now" for ease
                // of testing
                if property == "modified" || property == "end" || property == "entry" {
                    if value.is_some() {
                        value = Some("just-now".into());
                    }
                    if old_value.is_some() {
                        old_value = Some("just-now".into());
                    }
                }
                ReplicaOp::Update {
                    uuid,
                    property,
                    old_value,
                    value,
                    timestamp: now,
                }
            } else {
                op
            }
        };
        assert_eq!(
            rep.taskdb
                .operations()
                .drain(..)
                .map(clean_op)
                .collect::<Vec<_>>(),
            vec![
                ReplicaOp::UndoPoint,
                ReplicaOp::Create { uuid: t.get_uuid() },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "modified".into(),
                    old_value: None,
                    value: Some("just-now".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "description".into(),
                    old_value: None,
                    value: Some("a task".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "status".into(),
                    old_value: None,
                    value: Some("pending".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "entry".into(),
                    old_value: None,
                    value: Some("just-now".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "modified".into(),
                    old_value: Some("just-now".into()),
                    value: Some("just-now".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "description".into(),
                    old_value: Some("a task".into()),
                    value: Some("past tense".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "end".into(),
                    old_value: None,
                    value: Some("just-now".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid: t.get_uuid(),
                    property: "status".into(),
                    old_value: Some("pending".into()),
                    value: Some("completed".into()),
                    timestamp: now,
                },
            ]
        );

        assert_eq!(rep.num_local_operations().unwrap(), 10);
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

    #[test]
    fn expire() {
        let mut rep = Replica::new_inmemory();
        let mut t;

        rep.new_task(Status::Pending, "keeper 1".into()).unwrap();
        rep.new_task(Status::Completed, "keeper 2".into()).unwrap();

        t = rep.new_task(Status::Deleted, "keeper 3".into()).unwrap();
        {
            let mut t = t.into_mut(&mut rep);
            // set entry, with modification set as a side-effect
            t.set_entry(Some(Utc::now())).unwrap();
        }

        t = rep.new_task(Status::Deleted, "goner".into()).unwrap();
        {
            let mut t = t.into_mut(&mut rep);
            t.set_modified(Utc.ymd(1980, 1, 1).and_hms(0, 0, 0))
                .unwrap();
        }

        rep.expire_tasks().unwrap();

        for (_, t) in rep.all_tasks().unwrap() {
            println!("got task {}", t.get_description());
            assert!(t.get_description().starts_with("keeper"));
        }
    }

    #[test]
    fn dependency_map() {
        let mut rep = Replica::new_inmemory();

        let mut tasks = vec![];
        for _ in 0..4 {
            tasks.push(rep.new_task(Status::Pending, "t".into()).unwrap());
        }

        let uuids: Vec<_> = tasks.iter().map(|t| t.get_uuid()).collect();

        // t[3] depends on t[2], and t[1]
        {
            let mut t = tasks.pop().unwrap().into_mut(&mut rep);
            t.add_dependency(uuids[2]).unwrap();
            t.add_dependency(uuids[1]).unwrap();
        }

        // t[2] depends on t[0]
        {
            let mut t = tasks.pop().unwrap().into_mut(&mut rep);
            t.add_dependency(uuids[0]).unwrap();
        }

        // t[1] depends on t[0]
        {
            let mut t = tasks.pop().unwrap().into_mut(&mut rep);
            t.add_dependency(uuids[0]).unwrap();
        }

        // generate the dependency map, forcing an update based on the newly-added
        // dependencies
        let dm = rep.dependency_map(true).unwrap();

        assert_eq!(
            dm.dependencies(uuids[3]).collect::<HashSet<_>>(),
            set![uuids[1], uuids[2]]
        );
        assert_eq!(
            dm.dependencies(uuids[2]).collect::<HashSet<_>>(),
            set![uuids[0]]
        );
        assert_eq!(
            dm.dependencies(uuids[1]).collect::<HashSet<_>>(),
            set![uuids[0]]
        );
        assert_eq!(dm.dependencies(uuids[0]).collect::<HashSet<_>>(), set![]);

        assert_eq!(dm.dependents(uuids[3]).collect::<HashSet<_>>(), set![]);
        assert_eq!(
            dm.dependents(uuids[2]).collect::<HashSet<_>>(),
            set![uuids[3]]
        );
        assert_eq!(
            dm.dependents(uuids[1]).collect::<HashSet<_>>(),
            set![uuids[3]]
        );
        assert_eq!(
            dm.dependents(uuids[0]).collect::<HashSet<_>>(),
            set![uuids[1], uuids[2]]
        );
    }
}

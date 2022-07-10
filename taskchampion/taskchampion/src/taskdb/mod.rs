use crate::server::{Server, SyncOp};
use crate::storage::{ReplicaOp, Storage, TaskMap};
use uuid::Uuid;

mod apply;
mod snapshot;
mod sync;
mod undo;
mod working_set;

/// A TaskDb is the backend for a replica.  It manages the storage, operations, synchronization,
/// and so on, and all the invariants that come with it.  It leaves the meaning of particular task
/// properties to the replica and task implementations.
pub struct TaskDb {
    storage: Box<dyn Storage>,
}

impl TaskDb {
    /// Create a new TaskDb with the given backend storage
    pub fn new(storage: Box<dyn Storage>) -> TaskDb {
        TaskDb { storage }
    }

    #[cfg(test)]
    pub fn new_inmemory() -> TaskDb {
        #[cfg(test)]
        use crate::storage::InMemoryStorage;

        TaskDb::new(Box::new(InMemoryStorage::new()))
    }

    /// Apply an operation to the TaskDb.  This will update the set of tasks and add a ReplicaOp to
    /// the set of operations in the TaskDb, and return the TaskMap containing the resulting task's
    /// properties (or an empty TaskMap for deletion).
    ///
    /// Aside from synchronization operations, this is the only way to modify the TaskDb.  In cases
    /// where an operation does not make sense, this function will do nothing and return an error
    /// (but leave the TaskDb in a consistent state).
    pub fn apply(&mut self, op: SyncOp) -> anyhow::Result<TaskMap> {
        let mut txn = self.storage.txn()?;
        apply::apply_and_record(txn.as_mut(), op)
    }

    /// Add an UndoPoint operation to the list of replica operations.
    pub fn add_undo_point(&mut self) -> anyhow::Result<()> {
        let mut txn = self.storage.txn()?;
        txn.add_operation(ReplicaOp::UndoPoint)?;
        txn.commit()
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
        working_set::rebuild(self.storage.txn()?.as_mut(), in_working_set, renumber)
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
        let mut txn = self.storage.txn()?;
        sync::sync(server, txn.as_mut(), avoid_snapshots)
    }

    /// Undo local operations until the most recent UndoPoint, returning false if there are no
    /// local operations to undo.
    pub fn undo(&mut self) -> anyhow::Result<bool> {
        let mut txn = self.storage.txn()?;
        undo::undo(txn.as_mut())
    }

    /// Get the number of un-synchronized operations in storage.
    pub fn num_operations(&mut self) -> anyhow::Result<usize> {
        let mut txn = self.storage.txn().unwrap();
        txn.num_operations()
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
    pub(crate) fn operations(&mut self) -> Vec<ReplicaOp> {
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
    use crate::storage::{InMemoryStorage, ReplicaOp};
    use chrono::Utc;
    use pretty_assertions::assert_eq;
    use proptest::prelude::*;
    use uuid::Uuid;

    #[test]
    fn test_apply() {
        // this verifies that the operation is both applied and included in the list of
        // operations; more detailed tests are in the `apply` module.
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = SyncOp::Create { uuid };
        db.apply(op.clone()).unwrap();

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![]),]);
        assert_eq!(db.operations(), vec![ReplicaOp::Create { uuid }]);
    }

    #[test]
    fn test_add_undo_point() {
        let mut db = TaskDb::new_inmemory();
        db.add_undo_point().unwrap();
        assert_eq!(db.operations(), vec![ReplicaOp::UndoPoint]);
    }

    fn newdb() -> TaskDb {
        TaskDb::new(Box::new(InMemoryStorage::new()))
    }

    #[derive(Debug)]
    enum Action {
        Op(SyncOp),
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
                        b'C' => Action::Op(SyncOp::Create { uuid }),
                        b'U' => Action::Op(SyncOp::Update {
                            uuid,
                            property: "title".into(),
                            value: Some("foo".into()),
                            timestamp: Utc::now(),
                        }),
                        b'D' => Action::Op(SyncOp::Delete { uuid }),
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
                    Action::Sync => db.sync(&mut server, false).unwrap(),
                }
            }

            assert_eq!(dbs[0].sorted_tasks(), dbs[0].sorted_tasks());
            assert_eq!(dbs[1].sorted_tasks(), dbs[2].sorted_tasks());
        }
    }
}

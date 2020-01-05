use crate::operation::Operation;
use crate::taskstorage::{TaskMap, TaskStorage};
use failure::Fallible;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use uuid::Uuid;

#[derive(PartialEq, Debug, Clone)]
pub struct InMemoryStorage {
    // The current state, with all pending operations applied
    tasks: HashMap<Uuid, TaskMap>,

    // The version at which `operations` begins
    base_version: u64,

    // Operations applied since `base_version`, in order.
    //
    // INVARIANT: Given a snapshot at `base_version`, applying these operations produces `tasks`.
    operations: Vec<Operation>,
}

impl InMemoryStorage {
    pub fn new() -> InMemoryStorage {
        InMemoryStorage {
            tasks: HashMap::new(),
            base_version: 0,
            operations: vec![],
        }
    }
}

impl TaskStorage for InMemoryStorage {
    /// Get an (immutable) task, if it is in the storage
    fn get_task(&self, uuid: &Uuid) -> Fallible<Option<TaskMap>> {
        match self.tasks.get(uuid) {
            None => Ok(None),
            Some(t) => Ok(Some(t.clone())),
        }
    }

    /// Create a task, only if it does not already exist.  Returns true if
    /// the task was created (did not already exist).
    fn create_task(&mut self, uuid: Uuid, task: TaskMap) -> Fallible<bool> {
        if let ent @ Entry::Vacant(_) = self.tasks.entry(uuid) {
            ent.or_insert(task);
            Ok(true)
        } else {
            Ok(false)
        }
    }

    /// Set a task, overwriting any existing task.
    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> Fallible<()> {
        self.tasks.insert(uuid, task);
        Ok(())
    }

    /// Delete a task, if it exists.  Returns true if the task was deleted (already existed)
    fn delete_task(&mut self, uuid: &Uuid) -> Fallible<bool> {
        if let Some(_) = self.tasks.remove(uuid) {
            Ok(true)
        } else {
            Ok(false)
        }
    }

    fn get_task_uuids<'a>(&'a self) -> Fallible<Box<dyn Iterator<Item = Uuid> + 'a>> {
        Ok(Box::new(self.tasks.keys().map(|u| u.clone())))
    }

    /// Add an operation to the list of operations in the storage.  Note that this merely *stores*
    /// the operation; it is up to the TaskDB to apply it.
    fn add_operation(&mut self, op: Operation) -> Fallible<()> {
        self.operations.push(op);
        Ok(())
    }

    /// Get the current base_version for this storage -- the last version synced from the server.
    fn base_version(&self) -> Fallible<u64> {
        Ok(self.base_version)
    }

    /// Get the current set of outstanding operations (operations that have not been sync'd to the
    /// server yet)
    fn operations<'a>(&'a self) -> Fallible<Box<dyn Iterator<Item = &'a Operation> + 'a>> {
        Ok(Box::new(self.operations.iter()))
    }

    /// Apply the next version from the server.  This replaces the existing base_version and
    /// operations.  It's up to the caller (TaskDB) to ensure this is done consistently.
    fn update_version(&mut self, version: u64, new_operations: Vec<Operation>) -> Fallible<()> {
        // ensure that we are applying the versions in order..
        assert_eq!(version, self.base_version + 1);
        self.base_version = version;
        self.operations = new_operations;
        Ok(())
    }

    /// Record the outstanding operations as synced to the server in the given version.
    fn local_operations_synced(&mut self, version: u64) -> Fallible<()> {
        assert_eq!(version, self.base_version + 1);
        self.base_version = version;
        self.operations = vec![];
        Ok(())
    }
}

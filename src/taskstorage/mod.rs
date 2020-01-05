use crate::Operation;
use std::collections::HashMap;
use std::fmt;
use uuid::Uuid;

mod inmemory;

pub use inmemory::InMemoryStorage;

/// An in-memory representation of a task as a simple hashmap
pub type TaskMap = HashMap<String, String>;

/// A trait for objects able to act as backing storage for a TaskDB.  This API is optimized to be
/// easy to implement, with all of the semantic meaning of the data located in the TaskDB
/// implementation, which is the sole consumer of this trait.
pub trait TaskStorage: fmt::Debug {
    /// Get an (immutable) task, if it is in the storage
    fn get_task(&self, uuid: &Uuid) -> Option<TaskMap>;

    /// Create a task, only if it does not already exist.  Returns true if
    /// the task was created (did not already exist).
    fn create_task(&mut self, uuid: Uuid, task: TaskMap) -> bool;

    /// Set a task, overwriting any existing task.
    fn set_task(&mut self, uuid: Uuid, task: TaskMap);

    /// Delete a task, if it exists.  Returns true if the task was deleted (already existed)
    fn delete_task(&mut self, uuid: &Uuid) -> bool;

    /// Get the uuids of all tasks in the storage, in undefined order.
    fn get_task_uuids<'a>(&'a self) -> Box<dyn Iterator<Item = Uuid> + 'a>;

    /// Add an operation to the list of operations in the storage.  Note that this merely *stores*
    /// the operation; it is up to the TaskDB to apply it.
    fn add_operation(&mut self, op: Operation);

    /// Get the current base_version for this storage -- the last version synced from the server.
    fn base_version(&self) -> u64;

    /// Get the current set of outstanding operations (operations that have not been sync'd to the
    /// server yet)
    fn operations<'a>(&'a self) -> Box<dyn Iterator<Item = &Operation> + 'a>;

    /// Apply the next version from the server.  This replaces the existing base_version and
    /// operations.  It's up to the caller (TaskDB) to ensure this is done consistently.
    fn update_version(&mut self, version: u64, new_operations: Vec<Operation>);

    /// Record the outstanding operations as synced to the server in the given version.
    fn local_operations_synced(&mut self, version: u64);
}

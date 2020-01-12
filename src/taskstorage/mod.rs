use crate::Operation;
use failure::Fallible;
use std::collections::HashMap;
use uuid::Uuid;

mod inmemory;
mod kv;

pub use inmemory::InMemoryStorage;

/// An in-memory representation of a task as a simple hashmap
pub type TaskMap = HashMap<String, String>;

#[cfg(test)]
fn taskmap_with(mut properties: Vec<(String, String)>) -> TaskMap {
    let mut rv = TaskMap::new();
    for (p, v) in properties.drain(..) {
        rv.insert(p, v);
    }
    rv
}

/// A TaskStorage transaction, in which storage operations are performed.
/// Serializable consistency is maintained, and implementations do not optimize
/// for concurrent access so some may simply apply a mutex to limit access to
/// one transaction at a time.  Transactions are aborted if they are dropped.
/// It's safe to drop transactions that did not modify any data.
pub trait TaskStorageTxn {
    /// Get an (immutable) task, if it is in the storage
    fn get_task(&mut self, uuid: &Uuid) -> Fallible<Option<TaskMap>>;

    /// Create an (empty) task, only if it does not already exist.  Returns true if
    /// the task was created (did not already exist).
    fn create_task(&mut self, uuid: Uuid) -> Fallible<bool>;

    /// Set a task, overwriting any existing task.  If the task does not exist, this implicitly
    /// creates it (use `get_task` to check first, if necessary).
    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> Fallible<()>;

    /// Delete a task, if it exists.  Returns true if the task was deleted (already existed)
    fn delete_task(&mut self, uuid: &Uuid) -> Fallible<bool>;

    /// Get the uuids and bodies of all tasks in the storage, in undefined order.
    fn all_tasks<'a>(&mut self) -> Fallible<Vec<(Uuid, TaskMap)>>;

    /// Get the uuids of all tasks in the storage, in undefined order.
    fn all_task_uuids<'a>(&mut self) -> Fallible<Vec<Uuid>>;

    /// Get the current base_version for this storage -- the last version synced from the server.
    fn base_version(&mut self) -> Fallible<u64>;

    /// Set the current base_version for this storage.
    fn set_base_version(&mut self, version: u64) -> Fallible<()>;

    /// Get the current set of outstanding operations (operations that have not been sync'd to the
    /// server yet)
    fn operations<'a>(&mut self) -> Fallible<Vec<Operation>>;

    /// Add an operation to the end of the list of operations in the storage.  Note that this
    /// merely *stores* the operation; it is up to the DB to apply it.
    fn add_operation(&mut self, op: Operation) -> Fallible<()>;

    /// Replace the current list of operations with a new list.
    fn set_operations(&mut self, ops: Vec<Operation>) -> Fallible<()>;

    /// Commit any changes made in the transaction.  It is an error to call this more than
    /// once.
    fn commit(&mut self) -> Fallible<()>;
}

/// A trait for objects able to act as backing storage for a DB.  This API is optimized to be
/// easy to implement, with all of the semantic meaning of the data located in the DB
/// implementation, which is the sole consumer of this trait.
///
/// Conceptually, task storage contains the following:
///
///  - tasks: a set of tasks indexed by uuid
///  - base_version: the number of the last version sync'd from the server
///  - operations: all operations performed since base_version
///
///  The `operations` are already reflected in `tasks`, so the following invariant holds:
///  > Applying `operations` to the set of tasks at `base_version` gives a set of tasks identical
///  > to `tasks`.
///
///  It is up to the caller (DB) to maintain this invariant.
pub trait TaskStorage {
    /// Begin a transaction
    fn txn<'a>(&'a mut self) -> Fallible<Box<dyn TaskStorageTxn + 'a>>;
}

use serde::{Deserialize, Serialize};
use uuid::Uuid;

#[cfg(debug_assertions)]
mod inmemory;

#[cfg(debug_assertions)]
pub use inmemory::InMemoryStorage;

mod sqlite;
pub use self::sqlite::SqliteStorage;

#[derive(Clone, PartialEq, Debug, Serialize, Deserialize)]
pub struct Client {
    pub latest_version_id: Uuid,
}

#[derive(Clone, PartialEq, Debug, Serialize, Deserialize)]
pub struct Version {
    pub version_id: Uuid,
    pub parent_version_id: Uuid,
    pub history_segment: Vec<u8>,
}

pub trait StorageTxn {
    /// Get information about the given client
    fn get_client(&mut self, client_key: Uuid) -> anyhow::Result<Option<Client>>;

    /// Create a new client with the given latest_version_id
    fn new_client(&mut self, client_key: Uuid, latest_version_id: Uuid) -> anyhow::Result<()>;

    /// Set the client's latest_version_id
    fn set_client_latest_version_id(
        &mut self,
        client_key: Uuid,
        latest_version_id: Uuid,
    ) -> anyhow::Result<()>;

    /// Get a version, indexed by parent version id
    fn get_version_by_parent(
        &mut self,
        client_key: Uuid,
        parent_version_id: Uuid,
    ) -> anyhow::Result<Option<Version>>;

    /// Add a version (that must not already exist)
    fn add_version(
        &mut self,
        client_key: Uuid,
        version_id: Uuid,
        parent_version_id: Uuid,
        history_segment: Vec<u8>,
    ) -> anyhow::Result<()>;

    /// Commit any changes made in the transaction.  It is an error to call this more than
    /// once.  It is safe to skip this call for read-only operations.
    fn commit(&mut self) -> anyhow::Result<()>;
}

/// A trait for objects able to act as storage.  Most of the interesting behavior is in the
/// [`crate::storage::StorageTxn`] trait.
pub trait Storage: Send + Sync {
    /// Begin a transaction
    fn txn<'a>(&'a self) -> anyhow::Result<Box<dyn StorageTxn + 'a>>;
}

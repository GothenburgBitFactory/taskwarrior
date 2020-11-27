use failure::Fallible;
use taskchampion::Uuid;

mod inmemory;
pub(crate) use inmemory::InMemoryStorage;

#[derive(Clone)]
pub(crate) struct Client {
    pub(crate) latest_version_id: Uuid,
}

#[derive(Clone)]
pub(crate) struct Version {
    pub(crate) version_id: Uuid,
    pub(crate) parent_version_id: Uuid,
    pub(crate) history_segment: Vec<u8>,
}

pub(crate) trait StorageTxn {
    /// Get information about the given client
    fn get_client(&mut self, client_id: Uuid) -> Fallible<Option<Client>>;

    /// Set the client's latest_version_id (creating the client if necessary)
    fn set_client_latest_version_id(
        &mut self,
        client_id: Uuid,
        latest_version_id: Uuid,
    ) -> Fallible<()>;

    /// Get a version, indexed by parent version id
    fn get_version_by_parent(
        &mut self,
        client_id: Uuid,
        parent_version_id: Uuid,
    ) -> Fallible<Option<Version>>;

    /// Add a version (that must not already exist)
    fn add_version(
        &mut self,
        client_id: Uuid,
        version_id: Uuid,
        parent_version_id: Uuid,
        history_segment: Vec<u8>,
    ) -> Fallible<()>;

    /// Commit any changes made in the transaction.  It is an error to call this more than
    /// once.  It is safe to skip this call for read-only operations.
    fn commit(&mut self) -> Fallible<()>;
}

/// A trait for objects able to act as storage.  Most of the interesting behavior is in the
/// [`crate::storage::StorageTxn`] trait.
pub(crate) trait Storage: Send + Sync {
    /// Begin a transaction
    fn txn<'a>(&'a self) -> Fallible<Box<dyn StorageTxn + 'a>>;
}

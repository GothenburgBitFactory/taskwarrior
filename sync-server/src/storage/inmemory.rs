use super::{Client, Storage, StorageTxn, Uuid, Version};
use failure::{format_err, Fallible};
use std::collections::HashMap;
use std::sync::{Mutex, MutexGuard};

struct Inner {
    /// Clients, indexed by client_id
    clients: HashMap<Uuid, Client>,

    /// Versions, indexed by (client_id, parent_version_id)
    versions: HashMap<(Uuid, Uuid), Version>,
}

pub(crate) struct InMemoryStorage(Mutex<Inner>);

impl InMemoryStorage {
    pub(crate) fn new() -> Self {
        Self(Mutex::new(Inner {
            clients: HashMap::new(),
            versions: HashMap::new(),
        }))
    }
}

struct InnerTxn<'a>(MutexGuard<'a, Inner>);

/// In-memory storage for testing and experimentation.
///
/// NOTE: this does not implement transaction rollback.
impl Storage for InMemoryStorage {
    fn txn<'a>(&'a self) -> Fallible<Box<dyn StorageTxn + 'a>> {
        Ok(Box::new(InnerTxn(self.0.lock().expect("poisoned lock"))))
    }
}

impl<'a> StorageTxn for InnerTxn<'a> {
    fn get_client(&mut self, client_id: Uuid) -> Fallible<Option<Client>> {
        Ok(self.0.clients.get(&client_id).cloned())
    }

    fn new_client(&mut self, client_id: Uuid, latest_version_id: Uuid) -> Fallible<()> {
        if let Some(_) = self.0.clients.get(&client_id) {
            return Err(format_err!("Client {} already exists", client_id));
        }
        self.0
            .clients
            .insert(client_id, Client { latest_version_id });
        Ok(())
    }

    fn set_client_latest_version_id(
        &mut self,
        client_id: Uuid,
        latest_version_id: Uuid,
    ) -> Fallible<()> {
        if let Some(client) = self.0.clients.get_mut(&client_id) {
            Ok(client.latest_version_id = latest_version_id)
        } else {
            Err(format_err!("Client {} does not exist", client_id))
        }
    }

    fn get_version_by_parent(
        &mut self,
        client_id: Uuid,
        parent_version_id: Uuid,
    ) -> Fallible<Option<Version>> {
        Ok(self
            .0
            .versions
            .get(&(client_id, parent_version_id))
            .cloned())
    }

    fn add_version(
        &mut self,
        client_id: Uuid,
        version_id: Uuid,
        parent_version_id: Uuid,
        history_segment: Vec<u8>,
    ) -> Fallible<()> {
        // TODO: verify it doesn't exist (`.entry`?)
        let version = Version {
            version_id,
            parent_version_id,
            history_segment,
        };
        self.0
            .versions
            .insert((client_id, version.parent_version_id), version);
        Ok(())
    }

    fn commit(&mut self) -> Fallible<()> {
        Ok(())
    }
}

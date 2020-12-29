use super::{Client, Storage, StorageTxn, Uuid, Version};
use failure::{format_err, Fallible};
use std::collections::HashMap;
use std::sync::{Mutex, MutexGuard};

struct Inner {
    /// Clients, indexed by client_key
    clients: HashMap<Uuid, Client>,

    /// Versions, indexed by (client_key, parent_version_id)
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
    fn get_client(&mut self, client_key: Uuid) -> Fallible<Option<Client>> {
        Ok(self.0.clients.get(&client_key).cloned())
    }

    fn new_client(&mut self, client_key: Uuid, latest_version_id: Uuid) -> Fallible<()> {
        if self.0.clients.get(&client_key).is_some() {
            return Err(format_err!("Client {} already exists", client_key));
        }
        self.0
            .clients
            .insert(client_key, Client { latest_version_id });
        Ok(())
    }

    fn set_client_latest_version_id(
        &mut self,
        client_key: Uuid,
        latest_version_id: Uuid,
    ) -> Fallible<()> {
        if let Some(client) = self.0.clients.get_mut(&client_key) {
            client.latest_version_id = latest_version_id;
            Ok(())
        } else {
            Err(format_err!("Client {} does not exist", client_key))
        }
    }

    fn get_version_by_parent(
        &mut self,
        client_key: Uuid,
        parent_version_id: Uuid,
    ) -> Fallible<Option<Version>> {
        Ok(self
            .0
            .versions
            .get(&(client_key, parent_version_id))
            .cloned())
    }

    fn add_version(
        &mut self,
        client_key: Uuid,
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
            .insert((client_key, version.parent_version_id), version);
        Ok(())
    }

    fn commit(&mut self) -> Fallible<()> {
        Ok(())
    }
}

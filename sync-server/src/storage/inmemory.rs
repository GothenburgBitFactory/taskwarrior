use super::{Client, Storage, StorageTxn, Uuid, Version};
use std::collections::HashMap;
use std::sync::{Mutex, MutexGuard};

struct Inner {
    /// Clients, indexed by client_key
    clients: HashMap<Uuid, Client>,

    /// Versions, indexed by (client_key, parent_version_id)
    versions: HashMap<(Uuid, Uuid), Version>,
}

pub struct InMemoryStorage(Mutex<Inner>);

impl InMemoryStorage {
    #[allow(clippy::new_without_default)]
    pub fn new() -> Self {
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
    fn txn<'a>(&'a self) -> anyhow::Result<Box<dyn StorageTxn + 'a>> {
        Ok(Box::new(InnerTxn(self.0.lock().expect("poisoned lock"))))
    }
}

impl<'a> StorageTxn for InnerTxn<'a> {
    fn get_client(&mut self, client_key: Uuid) -> anyhow::Result<Option<Client>> {
        Ok(self.0.clients.get(&client_key).cloned())
    }

    fn new_client(&mut self, client_key: Uuid, latest_version_id: Uuid) -> anyhow::Result<()> {
        if self.0.clients.get(&client_key).is_some() {
            return Err(anyhow::anyhow!("Client {} already exists", client_key));
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
    ) -> anyhow::Result<()> {
        if let Some(client) = self.0.clients.get_mut(&client_key) {
            client.latest_version_id = latest_version_id;
            Ok(())
        } else {
            Err(anyhow::anyhow!("Client {} does not exist", client_key))
        }
    }

    fn get_version_by_parent(
        &mut self,
        client_key: Uuid,
        parent_version_id: Uuid,
    ) -> anyhow::Result<Option<Version>> {
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
    ) -> anyhow::Result<()> {
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

    fn commit(&mut self) -> anyhow::Result<()> {
        Ok(())
    }
}

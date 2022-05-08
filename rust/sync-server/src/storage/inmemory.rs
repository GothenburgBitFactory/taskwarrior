use super::{Client, Snapshot, Storage, StorageTxn, Uuid, Version};
use std::collections::HashMap;
use std::sync::{Mutex, MutexGuard};

struct Inner {
    /// Clients, indexed by client_key
    clients: HashMap<Uuid, Client>,

    /// Snapshot data, indexed by client key
    snapshots: HashMap<Uuid, Vec<u8>>,

    /// Versions, indexed by (client_key, version_id)
    versions: HashMap<(Uuid, Uuid), Version>,

    /// Child versions, indexed by (client_key, parent_version_id)
    children: HashMap<(Uuid, Uuid), Uuid>,
}

pub struct InMemoryStorage(Mutex<Inner>);

impl InMemoryStorage {
    #[allow(clippy::new_without_default)]
    pub fn new() -> Self {
        Self(Mutex::new(Inner {
            clients: HashMap::new(),
            snapshots: HashMap::new(),
            versions: HashMap::new(),
            children: HashMap::new(),
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
        self.0.clients.insert(
            client_key,
            Client {
                latest_version_id,
                snapshot: None,
            },
        );
        Ok(())
    }

    fn set_snapshot(
        &mut self,
        client_key: Uuid,
        snapshot: Snapshot,
        data: Vec<u8>,
    ) -> anyhow::Result<()> {
        let mut client = self
            .0
            .clients
            .get_mut(&client_key)
            .ok_or_else(|| anyhow::anyhow!("no such client"))?;
        client.snapshot = Some(snapshot);
        self.0.snapshots.insert(client_key, data);
        Ok(())
    }

    fn get_snapshot_data(
        &mut self,
        client_key: Uuid,
        version_id: Uuid,
    ) -> anyhow::Result<Option<Vec<u8>>> {
        // sanity check
        let client = self.0.clients.get(&client_key);
        let client = client.ok_or_else(|| anyhow::anyhow!("no such client"))?;
        if Some(&version_id) != client.snapshot.as_ref().map(|snap| &snap.version_id) {
            return Err(anyhow::anyhow!("unexpected snapshot_version_id"));
        }
        Ok(self.0.snapshots.get(&client_key).cloned())
    }

    fn get_version_by_parent(
        &mut self,
        client_key: Uuid,
        parent_version_id: Uuid,
    ) -> anyhow::Result<Option<Version>> {
        if let Some(parent_version_id) = self.0.children.get(&(client_key, parent_version_id)) {
            Ok(self
                .0
                .versions
                .get(&(client_key, *parent_version_id))
                .cloned())
        } else {
            Ok(None)
        }
    }

    fn get_version(
        &mut self,
        client_key: Uuid,
        version_id: Uuid,
    ) -> anyhow::Result<Option<Version>> {
        Ok(self.0.versions.get(&(client_key, version_id)).cloned())
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

        if let Some(client) = self.0.clients.get_mut(&client_key) {
            client.latest_version_id = version_id;
            if let Some(ref mut snap) = client.snapshot {
                snap.versions_since += 1;
            }
        } else {
            return Err(anyhow::anyhow!("Client {} does not exist", client_key));
        }

        self.0
            .children
            .insert((client_key, parent_version_id), version_id);
        self.0.versions.insert((client_key, version_id), version);

        Ok(())
    }

    fn commit(&mut self) -> anyhow::Result<()> {
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use chrono::Utc;

    #[test]
    fn test_get_client_empty() -> anyhow::Result<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;
        let maybe_client = txn.get_client(Uuid::new_v4())?;
        assert!(maybe_client.is_none());
        Ok(())
    }

    #[test]
    fn test_client_storage() -> anyhow::Result<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;

        let client_key = Uuid::new_v4();
        let latest_version_id = Uuid::new_v4();
        txn.new_client(client_key, latest_version_id)?;

        let client = txn.get_client(client_key)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);
        assert!(client.snapshot.is_none());

        let latest_version_id = Uuid::new_v4();
        txn.add_version(client_key, latest_version_id, Uuid::new_v4(), vec![1, 1])?;

        let client = txn.get_client(client_key)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);
        assert!(client.snapshot.is_none());

        let snap = Snapshot {
            version_id: Uuid::new_v4(),
            timestamp: Utc::now(),
            versions_since: 4,
        };
        txn.set_snapshot(client_key, snap.clone(), vec![1, 2, 3])?;

        let client = txn.get_client(client_key)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);
        assert_eq!(client.snapshot.unwrap(), snap);

        Ok(())
    }

    #[test]
    fn test_gvbp_empty() -> anyhow::Result<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;
        let maybe_version = txn.get_version_by_parent(Uuid::new_v4(), Uuid::new_v4())?;
        assert!(maybe_version.is_none());
        Ok(())
    }

    #[test]
    fn test_add_version_and_get_version() -> anyhow::Result<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;

        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abc".to_vec();

        txn.new_client(client_key, parent_version_id)?;
        txn.add_version(
            client_key,
            version_id,
            parent_version_id,
            history_segment.clone(),
        )?;

        let expected = Version {
            version_id,
            parent_version_id,
            history_segment,
        };

        let version = txn
            .get_version_by_parent(client_key, parent_version_id)?
            .unwrap();
        assert_eq!(version, expected);

        let version = txn.get_version(client_key, version_id)?.unwrap();
        assert_eq!(version, expected);

        Ok(())
    }

    #[test]
    fn test_snapshots() -> anyhow::Result<()> {
        let storage = InMemoryStorage::new();
        let mut txn = storage.txn()?;

        let client_key = Uuid::new_v4();

        txn.new_client(client_key, Uuid::new_v4())?;
        assert!(txn.get_client(client_key)?.unwrap().snapshot.is_none());

        let snap = Snapshot {
            version_id: Uuid::new_v4(),
            timestamp: Utc::now(),
            versions_since: 3,
        };
        txn.set_snapshot(client_key, snap.clone(), vec![9, 8, 9])?;

        assert_eq!(
            txn.get_snapshot_data(client_key, snap.version_id)?.unwrap(),
            vec![9, 8, 9]
        );
        assert_eq!(txn.get_client(client_key)?.unwrap().snapshot, Some(snap));

        let snap2 = Snapshot {
            version_id: Uuid::new_v4(),
            timestamp: Utc::now(),
            versions_since: 10,
        };
        txn.set_snapshot(client_key, snap2.clone(), vec![0, 2, 4, 6])?;

        assert_eq!(
            txn.get_snapshot_data(client_key, snap2.version_id)?
                .unwrap(),
            vec![0, 2, 4, 6]
        );
        assert_eq!(txn.get_client(client_key)?.unwrap().snapshot, Some(snap2));

        // check that mismatched version is detected
        assert!(txn.get_snapshot_data(client_key, Uuid::new_v4()).is_err());

        Ok(())
    }
}

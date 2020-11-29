#![allow(dead_code, unused_variables)] // temporary
use super::{Client, Storage, StorageTxn, Uuid, Version};
use failure::Fallible;
use kv::msgpack::Msgpack;
use kv::{Bucket, Config, Error, Serde, Store, ValueBuf};
use std::path::Path;

/// Key for versions: concatenation of client_id and parent_version_id
type VersionKey = [u8; 32];

fn version_key(client_id: Uuid, parent_version_id: Uuid) -> VersionKey {
    let mut key = [0u8; 32];
    key[..16].clone_from_slice(client_id.as_bytes());
    key[16..].clone_from_slice(parent_version_id.as_bytes());
    key
}

/// Key for clients: just the client_id
type ClientKey = [u8; 16];

fn client_key(client_id: Uuid) -> ClientKey {
    *client_id.as_bytes()
}

/// KVStorage is an on-disk storage backend which uses LMDB via the `kv` crate.
pub(crate) struct KVStorage<'t> {
    store: Store,
    clients_bucket: Bucket<'t, ClientKey, ValueBuf<Msgpack<Client>>>,
    versions_bucket: Bucket<'t, VersionKey, ValueBuf<Msgpack<Version>>>,
}

impl<'t> KVStorage<'t> {
    pub fn new<P: AsRef<Path>>(directory: P) -> Fallible<KVStorage<'t>> {
        let mut config = Config::default(directory);
        config.bucket("clients", None);
        config.bucket("versions", None);

        let store = Store::new(config)?;

        let clients_bucket =
            store.bucket::<ClientKey, ValueBuf<Msgpack<Client>>>(Some("clients"))?;
        let versions_bucket =
            store.bucket::<VersionKey, ValueBuf<Msgpack<Version>>>(Some("versions"))?;

        Ok(KVStorage {
            store,
            clients_bucket,
            versions_bucket,
        })
    }
}

impl<'t> Storage for KVStorage<'t> {
    fn txn<'a>(&'a self) -> Fallible<Box<dyn StorageTxn + 'a>> {
        Ok(Box::new(Txn {
            storage: self,
            txn: Some(self.store.write_txn()?),
        }))
    }
}

struct Txn<'t> {
    storage: &'t KVStorage<'t>,
    txn: Option<kv::Txn<'t>>,
}

impl<'t> Txn<'t> {
    // get the underlying kv Txn
    fn kvtxn(&mut self) -> &mut kv::Txn<'t> {
        if let Some(ref mut txn) = self.txn {
            txn
        } else {
            panic!("cannot use transaction after commit");
        }
    }

    fn clients_bucket(&self) -> &'t Bucket<'t, ClientKey, ValueBuf<Msgpack<Client>>> {
        &self.storage.clients_bucket
    }
    fn versions_bucket(&self) -> &'t Bucket<'t, VersionKey, ValueBuf<Msgpack<Version>>> {
        &self.storage.versions_bucket
    }
}

impl<'t> StorageTxn for Txn<'t> {
    fn get_client(&mut self, client_id: Uuid) -> Fallible<Option<Client>> {
        let key = client_key(client_id);
        let bucket = self.clients_bucket();
        let kvtxn = self.kvtxn();

        let client = match kvtxn.get(&bucket, key) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(None),
            Err(e) => return Err(e.into()),
        }
        .inner()?
        .to_serde();
        Ok(Some(client))
    }

    fn new_client(&mut self, client_id: Uuid, latest_version_id: Uuid) -> Fallible<()> {
        let key = client_key(client_id);
        let bucket = self.clients_bucket();
        let kvtxn = self.kvtxn();
        let client = Client { latest_version_id };
        kvtxn.set(&bucket, key, Msgpack::to_value_buf(client)?)?;
        Ok(())
    }

    fn set_client_latest_version_id(
        &mut self,
        client_id: Uuid,
        latest_version_id: Uuid,
    ) -> Fallible<()> {
        // implementation is the same as new_client..
        self.new_client(client_id, latest_version_id)
    }

    fn get_version_by_parent(
        &mut self,
        client_id: Uuid,
        parent_version_id: Uuid,
    ) -> Fallible<Option<Version>> {
        let key = version_key(client_id, parent_version_id);
        let bucket = self.versions_bucket();
        let kvtxn = self.kvtxn();
        let version = match kvtxn.get(&bucket, key) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(None),
            Err(e) => return Err(e.into()),
        }
        .inner()?
        .to_serde();
        Ok(Some(version))
    }

    fn add_version(
        &mut self,
        client_id: Uuid,
        version_id: Uuid,
        parent_version_id: Uuid,
        history_segment: Vec<u8>,
    ) -> Fallible<()> {
        let key = version_key(client_id, parent_version_id);
        let bucket = self.versions_bucket();
        let kvtxn = self.kvtxn();
        let version = Version {
            version_id,
            parent_version_id,
            history_segment,
        };
        kvtxn.set(&bucket, key, Msgpack::to_value_buf(version)?)?;
        Ok(())
    }

    fn commit(&mut self) -> Fallible<()> {
        if let Some(kvtxn) = self.txn.take() {
            kvtxn.commit()?;
        } else {
            panic!("transaction already committed");
        }
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use failure::Fallible;
    use tempdir::TempDir;

    #[test]
    fn test_get_client_empty() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let storage = KVStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;
        let maybe_client = txn.get_client(Uuid::new_v4())?;
        assert!(maybe_client.is_none());
        Ok(())
    }

    #[test]
    fn test_client_storage() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let storage = KVStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;

        let client_id = Uuid::new_v4();
        let latest_version_id = Uuid::new_v4();
        txn.new_client(client_id, latest_version_id)?;

        let client = txn.get_client(client_id)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);

        let latest_version_id = Uuid::new_v4();
        txn.set_client_latest_version_id(client_id, latest_version_id)?;

        let client = txn.get_client(client_id)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);

        Ok(())
    }

    #[test]
    fn test_gvbp_empty() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let storage = KVStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;
        let maybe_version = txn.get_version_by_parent(Uuid::new_v4(), Uuid::new_v4())?;
        assert!(maybe_version.is_none());
        Ok(())
    }

    #[test]
    fn test_add_version_and_gvbp() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let storage = KVStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;

        let client_id = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abc".to_vec();
        txn.add_version(
            client_id,
            version_id,
            parent_version_id,
            history_segment.clone(),
        )?;
        let version = txn
            .get_version_by_parent(client_id, parent_version_id)?
            .unwrap();

        assert_eq!(
            version,
            Version {
                version_id,
                parent_version_id,
                history_segment,
            }
        );
        Ok(())
    }
}

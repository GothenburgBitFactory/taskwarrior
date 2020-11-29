use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, VersionId, NO_VERSION_ID,
};
use crate::utils::Key;
use failure::Fallible;
use kv::msgpack::Msgpack;
use kv::{Bucket, Config, Error, Integer, Serde, Store, ValueBuf};
use serde::{Deserialize, Serialize};
use std::path::Path;
use uuid::Uuid;

#[derive(Serialize, Deserialize, Debug)]
struct Version {
    version_id: VersionId,
    parent_version_id: VersionId,
    history_segment: HistorySegment,
}

pub struct LocalServer<'t> {
    store: Store,
    // NOTE: indexed by parent_version_id!
    versions_bucket: Bucket<'t, Key, ValueBuf<Msgpack<Version>>>,
    latest_version_bucket: Bucket<'t, Integer, ValueBuf<Msgpack<Uuid>>>,
}

impl<'t> LocalServer<'t> {
    /// A test server has no notion of clients, signatures, encryption, etc.
    pub fn new<P: AsRef<Path>>(directory: P) -> Fallible<LocalServer<'t>> {
        let mut config = Config::default(directory);
        config.bucket("versions", None);
        config.bucket("numbers", None);
        config.bucket("latest_version", None);
        config.bucket("operations", None);
        config.bucket("working_set", None);
        let store = Store::new(config)?;

        // versions are stored indexed by VersionId (uuid)
        let versions_bucket = store.bucket::<Key, ValueBuf<Msgpack<Version>>>(Some("versions"))?;

        // this bucket contains the latest version at key 0
        let latest_version_bucket =
            store.int_bucket::<ValueBuf<Msgpack<Uuid>>>(Some("latest_version"))?;

        Ok(LocalServer {
            store,
            versions_bucket,
            latest_version_bucket,
        })
    }

    fn get_latest_version_id(&mut self) -> Fallible<VersionId> {
        let txn = self.store.read_txn()?;
        let base_version = match txn.get(&self.latest_version_bucket, 0.into()) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(NO_VERSION_ID),
            Err(e) => return Err(e.into()),
        }
        .inner()?
        .to_serde();
        Ok(base_version as VersionId)
    }

    fn set_latest_version_id(&mut self, version_id: VersionId) -> Fallible<()> {
        let mut txn = self.store.write_txn()?;
        txn.set(
            &self.latest_version_bucket,
            0.into(),
            Msgpack::to_value_buf(version_id as Uuid)?,
        )?;
        txn.commit()?;
        Ok(())
    }

    fn get_version_by_parent_version_id(
        &mut self,
        parent_version_id: VersionId,
    ) -> Fallible<Option<Version>> {
        let txn = self.store.read_txn()?;

        let version = match txn.get(&self.versions_bucket, parent_version_id.into()) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(None),
            Err(e) => return Err(e.into()),
        }
        .inner()?
        .to_serde();
        Ok(Some(version))
    }

    fn add_version_by_parent_version_id(&mut self, version: Version) -> Fallible<()> {
        let mut txn = self.store.write_txn()?;
        txn.set(
            &self.versions_bucket,
            version.parent_version_id.into(),
            Msgpack::to_value_buf(version)?,
        )?;
        txn.commit()?;
        Ok(())
    }
}

impl<'t> Server for LocalServer<'t> {
    // TODO: better transaction isolation for add_version (gets and sets should be in the same
    // transaction)

    /// Add a new version.  If the given version number is incorrect, this responds with the
    /// appropriate version and expects the caller to try again.
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult> {
        // no client lookup
        // no signature validation

        // check the parent_version_id for linearity
        let latest_version_id = self.get_latest_version_id()?;
        if latest_version_id != NO_VERSION_ID && parent_version_id != latest_version_id {
            return Ok(AddVersionResult::ExpectedParentVersion(latest_version_id));
        }

        // invent a new ID for this version
        let version_id = Uuid::new_v4();

        self.add_version_by_parent_version_id(Version {
            version_id,
            parent_version_id,
            history_segment,
        })?;
        self.set_latest_version_id(version_id)?;

        Ok(AddVersionResult::Ok(version_id))
    }

    /// Get a vector of all versions after `since_version`
    fn get_child_version(&mut self, parent_version_id: VersionId) -> Fallible<GetVersionResult> {
        if let Some(version) = self.get_version_by_parent_version_id(parent_version_id)? {
            Ok(GetVersionResult::Version {
                version_id: version.version_id,
                parent_version_id: version.parent_version_id,
                history_segment: version.history_segment,
            })
        } else {
            Ok(GetVersionResult::NoSuchVersion)
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use failure::Fallible;
    use tempdir::TempDir;

    #[test]
    fn test_empty() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let child_version = server.get_child_version(NO_VERSION_ID)?;
        assert_eq!(child_version, GetVersionResult::NoSuchVersion);
        Ok(())
    }

    #[test]
    fn test_add_zero_base() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        match server.add_version(NO_VERSION_ID, history.clone())? {
            AddVersionResult::ExpectedParentVersion(_) => {
                panic!("should have accepted the version")
            }
            AddVersionResult::Ok(version_id) => {
                let new_version = server.get_child_version(NO_VERSION_ID)?;
                assert_eq!(
                    new_version,
                    GetVersionResult::Version {
                        version_id,
                        parent_version_id: NO_VERSION_ID,
                        history_segment: history,
                    }
                );
            }
        }

        Ok(())
    }

    #[test]
    fn test_add_nonzero_base() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        let parent_version_id = Uuid::new_v4() as VersionId;

        // This is OK because the server has no latest_version_id yet
        match server.add_version(parent_version_id, history.clone())? {
            AddVersionResult::ExpectedParentVersion(_) => {
                panic!("should have accepted the version")
            }
            AddVersionResult::Ok(version_id) => {
                let new_version = server.get_child_version(parent_version_id)?;
                assert_eq!(
                    new_version,
                    GetVersionResult::Version {
                        version_id,
                        parent_version_id,
                        history_segment: history,
                    }
                );
            }
        }

        Ok(())
    }

    #[test]
    fn test_add_nonzero_base_forbidden() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        let parent_version_id = Uuid::new_v4() as VersionId;

        // add a version
        if let AddVersionResult::ExpectedParentVersion(_) =
            server.add_version(parent_version_id, history.clone())?
        {
            panic!("should have accepted the version")
        }

        // then add another, not based on that one
        if let AddVersionResult::Ok(_) = server.add_version(parent_version_id, history.clone())? {
            panic!("should not have accepted the version")
        }

        Ok(())
    }
}

use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, Snapshot, SnapshotUrgency,
    VersionId, NIL_VERSION_ID,
};
use crate::storage::sqlite::StoredUuid;
use anyhow::Context;
use rusqlite::params;
use rusqlite::OptionalExtension;
use serde::{Deserialize, Serialize};
use std::path::Path;
use uuid::Uuid;

#[derive(Serialize, Deserialize, Debug)]
struct Version {
    version_id: VersionId,
    parent_version_id: VersionId,
    history_segment: HistorySegment,
}

pub struct LocalServer {
    con: rusqlite::Connection,
}

impl LocalServer {
    fn txn(&mut self) -> anyhow::Result<rusqlite::Transaction> {
        let txn = self.con.transaction()?;
        Ok(txn)
    }

    /// A server which has no notion of clients, signatures, encryption, etc.
    pub fn new<P: AsRef<Path>>(directory: P) -> anyhow::Result<LocalServer> {
        let db_file = directory
            .as_ref()
            .join("taskchampion-local-sync-server.sqlite3");
        let con = rusqlite::Connection::open(&db_file)?;

        let queries = vec![
            "CREATE TABLE IF NOT EXISTS data (key STRING PRIMARY KEY, value STRING);",
            "CREATE TABLE IF NOT EXISTS versions (version_id STRING PRIMARY KEY, parent_version_id STRING, data STRING);",
        ];
        for q in queries {
            con.execute(q, []).context("Creating table")?;
        }

        Ok(LocalServer { con })
    }

    fn get_latest_version_id(&mut self) -> anyhow::Result<VersionId> {
        let t = self.txn()?;
        let result: Option<StoredUuid> = t
            .query_row(
                "SELECT value FROM data WHERE key = 'latest_version_id' LIMIT 1",
                rusqlite::params![],
                |r| r.get(0),
            )
            .optional()?;
        Ok(result.map(|x| x.0).unwrap_or(NIL_VERSION_ID))
    }

    fn set_latest_version_id(&mut self, version_id: VersionId) -> anyhow::Result<()> {
        let t = self.txn()?;
        t.execute(
            "INSERT OR REPLACE INTO data (key, value) VALUES ('latest_version_id', ?)",
            params![&StoredUuid(version_id)],
        )
        .context("Update task query")?;
        t.commit()?;
        Ok(())
    }

    fn get_version_by_parent_version_id(
        &mut self,
        parent_version_id: VersionId,
    ) -> anyhow::Result<Option<Version>> {
        let t = self.txn()?;
        let r = t.query_row(
            "SELECT version_id, parent_version_id, data FROM versions WHERE parent_version_id = ?",
            params![&StoredUuid(parent_version_id)],
            |r| {
                let version_id: StoredUuid = r.get("version_id")?;
                let parent_version_id: StoredUuid = r.get("parent_version_id")?;

                Ok(Version{
                version_id: version_id.0,
                parent_version_id: parent_version_id.0,
                history_segment: r.get("data")?,
            })}
            )
        .optional()
        .context("Get version query")
        ?;
        Ok(r)
    }

    fn add_version_by_parent_version_id(&mut self, version: Version) -> anyhow::Result<()> {
        let t = self.txn()?;
        t.execute(
            "INSERT INTO versions (version_id, parent_version_id, data) VALUES (?, ?, ?)",
            params![
                StoredUuid(version.version_id),
                StoredUuid(version.parent_version_id),
                version.history_segment
            ],
        )?;
        t.commit()?;
        Ok(())
    }
}

impl Server for LocalServer {
    // TODO: better transaction isolation for add_version (gets and sets should be in the same
    // transaction)

    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> anyhow::Result<(AddVersionResult, SnapshotUrgency)> {
        // no client lookup
        // no signature validation

        // check the parent_version_id for linearity
        let latest_version_id = self.get_latest_version_id()?;
        if latest_version_id != NIL_VERSION_ID && parent_version_id != latest_version_id {
            return Ok((
                AddVersionResult::ExpectedParentVersion(latest_version_id),
                SnapshotUrgency::None,
            ));
        }

        // invent a new ID for this version
        let version_id = Uuid::new_v4();

        self.add_version_by_parent_version_id(Version {
            version_id,
            parent_version_id,
            history_segment,
        })?;
        self.set_latest_version_id(version_id)?;

        Ok((AddVersionResult::Ok(version_id), SnapshotUrgency::None))
    }

    fn get_child_version(
        &mut self,
        parent_version_id: VersionId,
    ) -> anyhow::Result<GetVersionResult> {
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

    fn add_snapshot(&mut self, _version_id: VersionId, _snapshot: Snapshot) -> anyhow::Result<()> {
        // the local server never requests a snapshot, so it should never get one
        unreachable!()
    }

    fn get_snapshot(&mut self) -> anyhow::Result<Option<(VersionId, Snapshot)>> {
        Ok(None)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;
    use tempfile::TempDir;

    #[test]
    fn test_empty() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let child_version = server.get_child_version(NIL_VERSION_ID)?;
        assert_eq!(child_version, GetVersionResult::NoSuchVersion);
        Ok(())
    }

    #[test]
    fn test_add_zero_base() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        match server.add_version(NIL_VERSION_ID, history.clone())?.0 {
            AddVersionResult::ExpectedParentVersion(_) => {
                panic!("should have accepted the version")
            }
            AddVersionResult::Ok(version_id) => {
                let new_version = server.get_child_version(NIL_VERSION_ID)?;
                assert_eq!(
                    new_version,
                    GetVersionResult::Version {
                        version_id,
                        parent_version_id: NIL_VERSION_ID,
                        history_segment: history,
                    }
                );
            }
        }

        Ok(())
    }

    #[test]
    fn test_add_nonzero_base() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        let parent_version_id = Uuid::new_v4() as VersionId;

        // This is OK because the server has no latest_version_id yet
        match server.add_version(parent_version_id, history.clone())?.0 {
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
    fn test_add_nonzero_base_forbidden() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut server = LocalServer::new(&tmp_dir.path())?;
        let history = b"1234".to_vec();
        let parent_version_id = Uuid::new_v4() as VersionId;

        // add a version
        if let (AddVersionResult::ExpectedParentVersion(_), SnapshotUrgency::None) =
            server.add_version(parent_version_id, history.clone())?
        {
            panic!("should have accepted the version")
        }

        // then add another, not based on that one
        if let (AddVersionResult::Ok(_), SnapshotUrgency::None) =
            server.add_version(parent_version_id, history.clone())?
        {
            panic!("should not have accepted the version")
        }

        Ok(())
    }
}

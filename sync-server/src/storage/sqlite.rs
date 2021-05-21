use super::{Client, Storage, StorageTxn, Uuid, Version};
use anyhow::Context;
use rusqlite::types::{FromSql, ToSql};
use rusqlite::{params, Connection, OptionalExtension};
use std::path::Path;

#[derive(Debug, thiserror::Error)]
enum SqliteError {
    #[error("SQLite transaction already committted")]
    TransactionAlreadyCommitted,
}

/// Newtype to allow implementing `FromSql` for foreign `uuid::Uuid`
struct StoredUuid(Uuid);

/// Conversion from Uuid stored as a string (rusqlite's uuid feature stores as binary blob)
impl FromSql for StoredUuid {
    fn column_result(value: rusqlite::types::ValueRef<'_>) -> rusqlite::types::FromSqlResult<Self> {
        let u = Uuid::parse_str(value.as_str()?)
            .map_err(|_| rusqlite::types::FromSqlError::InvalidType)?;
        Ok(StoredUuid(u))
    }
}

/// Store Uuid as string in database
impl ToSql for StoredUuid {
    fn to_sql(&self) -> rusqlite::Result<rusqlite::types::ToSqlOutput<'_>> {
        let s = self.0.to_string();
        Ok(s.into())
    }
}

/// Stores [`Client`] in SQLite
impl FromSql for Client {
    fn column_result(value: rusqlite::types::ValueRef<'_>) -> rusqlite::types::FromSqlResult<Self> {
        let o: Client = serde_json::from_str(value.as_str()?)
            .map_err(|_| rusqlite::types::FromSqlError::InvalidType)?;
        Ok(o)
    }
}

/// Parsers Operation stored as JSON in string column
impl ToSql for Client {
    fn to_sql(&self) -> rusqlite::Result<rusqlite::types::ToSqlOutput<'_>> {
        let s = serde_json::to_string(&self)
            .map_err(|e| rusqlite::Error::ToSqlConversionFailure(Box::new(e)))?;
        Ok(s.into())
    }
}

/// DB Key for versions: concatenation of client_key and parent_version_id
type VersionDbKey = [u8; 32];

fn version_db_key(client_key: Uuid, parent_version_id: Uuid) -> VersionDbKey {
    let mut key = [0u8; 32];
    key[..16].clone_from_slice(client_key.as_bytes());
    key[16..].clone_from_slice(parent_version_id.as_bytes());
    key
}

/// Key for clients: just the client_key
type ClientDbKey = [u8; 16];

fn client_db_key(client_key: Uuid) -> ClientDbKey {
    *client_key.as_bytes()
}

/// An on-disk storage backend which uses SQLite
pub(crate) struct SqliteStorage {
    db_file: std::path::PathBuf,
}

impl SqliteStorage {
    fn new_connection(&self) -> anyhow::Result<Connection> {
        Ok(Connection::open(&self.db_file)?)
    }

    pub fn new<P: AsRef<Path>>(directory: P) -> anyhow::Result<SqliteStorage> {
        let db_file = directory.as_ref().join("taskchampion-sync-server.sqlite3");

        let o = SqliteStorage { db_file };

        {
            let mut con = o.new_connection()?;
            let txn = con.transaction()?;

            let queries = vec![
                "CREATE TABLE IF NOT EXISTS clients (client_key STRING PRIMARY KEY, latest_version_id STRING);",
                "CREATE TABLE IF NOT EXISTS versions (id STRING PRIMARY KEY, client_key STRING, parent STRING, history_segment STRING);",
            ];
            for q in queries {
                txn.execute(q, []).context("Creating table")?;
            }
            txn.commit()?;
        }

        Ok(o)
    }
}

impl Storage for SqliteStorage {
    fn txn<'a>(&'a self) -> anyhow::Result<Box<dyn StorageTxn + 'a>> {
        let con = self.new_connection()?;
        let t = Txn { con, txn: None };
        Ok(Box::new(t))
    }
}

struct Txn<'t> {
    con: Connection,
    txn: Option<rusqlite::Transaction<'t>>,
}

impl<'t> Txn<'t> {
    fn get_txn(&mut self) -> Result<rusqlite::Transaction, SqliteError> {
        Ok(self.con.transaction().unwrap())
    }
}

impl<'t> StorageTxn for Txn<'t> {
    fn get_client(&mut self, client_key: Uuid) -> anyhow::Result<Option<Client>> {
        let t = self.get_txn()?;
        let result: Option<Client> = t
            .query_row(
                "SELECT latest_version_id FROM clients WHERE client_key = ? LIMIT 1",
                [&StoredUuid(client_key)],
                |r| {
                    let latest_version_id: StoredUuid = r.get(0)?;
                    Ok(Client {
                        latest_version_id: latest_version_id.0,
                    })
                },
            )
            .optional()
            .context("Get client query")?;

        Ok(result)
    }

    fn new_client(&mut self, client_key: Uuid, latest_version_id: Uuid) -> anyhow::Result<()> {
        let t = self.get_txn()?;

        t.execute(
            "INSERT OR REPLACE INTO clients (client_key, latest_version_id) VALUES (?, ?)",
            params![&StoredUuid(client_key), &StoredUuid(latest_version_id)],
        )
        .context("Create client query")?;
        Ok(())
    }

    fn set_client_latest_version_id(
        &mut self,
        client_key: Uuid,
        latest_version_id: Uuid,
    ) -> anyhow::Result<()> {
        // Implementation is same as new_client
        self.new_client(client_key, latest_version_id)
    }

    fn get_version_by_parent(
        &mut self,
        client_key: Uuid,
        parent_version_id: Uuid,
    ) -> anyhow::Result<Option<Version>> {
        todo!()
    }

    fn add_version(
        &mut self,
        client_key: Uuid,
        version_id: Uuid,
        parent_version_id: Uuid,
        history_segment: Vec<u8>,
    ) -> anyhow::Result<()> {
        let t = self.get_txn()?;

        t.execute(
            "INSERT INTO versions (id, client_key, parent, history_segment) VALUES(?, ?, ?, ?)",
            params![
                StoredUuid(version_id),
                StoredUuid(client_key),
                StoredUuid(parent_version_id),
                history_segment
            ],
        )
        .context("Add version query")?;

        Ok(())
    }

    fn commit(&mut self) -> anyhow::Result<()> {
        let t: rusqlite::Transaction = self
            .txn
            .take()
            .ok_or(SqliteError::TransactionAlreadyCommitted)?;
        t.commit().context("Committing transaction")?;
        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use tempfile::TempDir;

    #[test]
    fn test_get_client_empty() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let storage = SqliteStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;
        let maybe_client = txn.get_client(Uuid::new_v4())?;
        assert!(maybe_client.is_none());
        Ok(())
    }

    #[test]
    fn test_client_storage() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let storage = SqliteStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;

        let client_key = Uuid::new_v4();
        let latest_version_id = Uuid::new_v4();
        txn.new_client(client_key, latest_version_id)?;

        let client = txn.get_client(client_key)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);

        let latest_version_id = Uuid::new_v4();
        txn.set_client_latest_version_id(client_key, latest_version_id)?;

        let client = txn.get_client(client_key)?.unwrap();
        assert_eq!(client.latest_version_id, latest_version_id);

        Ok(())
    }

    #[test]
    fn test_gvbp_empty() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let storage = SqliteStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;
        let maybe_version = txn.get_version_by_parent(Uuid::new_v4(), Uuid::new_v4())?;
        assert!(maybe_version.is_none());
        Ok(())
    }

    #[test]
    fn test_add_version_and_gvbp() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let storage = SqliteStorage::new(&tmp_dir.path())?;
        let mut txn = storage.txn()?;

        let client_key = Uuid::new_v4();
        let version_id = Uuid::new_v4();
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"abc".to_vec();
        txn.add_version(
            client_key,
            version_id,
            parent_version_id,
            history_segment.clone(),
        )?;
        let version = txn
            .get_version_by_parent(client_key, parent_version_id)?
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

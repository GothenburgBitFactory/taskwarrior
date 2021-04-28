use crate::storage::{Operation, Storage, StorageTxn, TaskMap, VersionId, DEFAULT_BASE_VERSION};
use crate::utils::Key;
use anyhow::Context;
use rusqlite::{params, Connection, OptionalExtension};
use serde::serde_if_integer128;
use std::path::Path;
use uuid::Uuid;

#[derive(Debug, thiserror::Error)]
enum SqliteError {
    #[error("SQLite transaction already committted")]
    TransactionAlreadyCommitted,
    #[error("Invalid UUID string from database: {0}")]
    InvalidUuidString(String),
}

/// SqliteStorage is an on-disk storage backed by SQLite3.
pub struct SqliteStorage {
    con: Connection,
}

impl SqliteStorage {
    pub fn new<P: AsRef<Path>>(directory: P) -> anyhow::Result<SqliteStorage> {
        let db_file = directory.as_ref().join("taskchampion.sqlite3");
        let con = Connection::open(db_file)?;

        let queries = vec![
            "CREATE TABLE IF NOT EXISTS operations (id INTEGER PRIMARY KEY AUTOINCREMENT, data STRING);",
            "CREATE TABLE IF NOT EXISTS sync_meta (key STRING PRIMARY KEY, value STRING);",
            "CREATE TABLE IF NOT EXISTS tasks (uuid STRING PRIMARY KEY, data STRING);",
            "CREATE TABLE IF NOT EXISTS working_set (id INTEGER PRIMARY KEY, uuid STRING);",
        ];
        for q in queries {
            con.execute(q, []).context("Creating table")?;
        }

        Ok(SqliteStorage { con })
    }
}

struct Txn<'t> {
    txn: Option<rusqlite::Transaction<'t>>,
}

impl<'t> Txn<'t> {
    fn get_txn(&self) -> Result<&rusqlite::Transaction<'t>, SqliteError> {
        self.txn
            .as_ref()
            .ok_or(SqliteError::TransactionAlreadyCommitted)
    }

    fn get_next_working_set_number(&self) -> anyhow::Result<usize> {
        let t = self.get_txn()?;
        let result: Option<usize> = t
            .query_row("SELECT COALESCE(MAX(id), 0) FROM working_set", [], |r| {
                r.get(0)
            })
            .optional()
            .context("Getting highest working set ID")?;

        Ok(result.unwrap_or(0) + 1)
    }
}

impl Storage for SqliteStorage {
    fn txn<'a>(&'a mut self) -> anyhow::Result<Box<dyn StorageTxn + 'a>> {
        let txn = self.con.transaction()?;
        Ok(Box::new(Txn { txn: Some(txn) }))
    }
}

impl<'t> StorageTxn for Txn<'t> {
    fn get_task(&mut self, uuid: Uuid) -> anyhow::Result<Option<TaskMap>> {
        let t = self.get_txn()?;
        let result: Option<String> = t
            .query_row(
                "SELECT data FROM tasks WHERE uuid = ? LIMIT 1",
                [&uuid],
                |r| r.get("data"),
            )
            .optional()?;

        match result {
            None => Ok(None),
            Some(r) => Ok(serde_json::from_str(&r)?),
        }
    }

    fn create_task(&mut self, uuid: Uuid) -> anyhow::Result<bool> {
        let t = self.get_txn()?;
        let count: usize = t.query_row(
            "SELECT count(uuid) FROM tasks WHERE uuid = ?",
            [&uuid],
            |x| x.get(0),
        )?;
        if count > 0 {
            return Ok(false);
        }

        let data = TaskMap::default();
        let data_str = serde_json::to_string(&data)?;
        t.execute(
            "INSERT INTO TASKS (uuid, data) VALUES (?, ?)",
            params![&uuid, &data_str],
        )
        .context("Create task query")?;
        Ok(true)
    }

    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> anyhow::Result<()> {
        let t = self.get_txn()?;
        let data_str = serde_json::to_string(&task)?;
        t.execute(
            "INSERT OR REPLACE INTO tasks (uuid, data) VALUES (?, ?)",
            params![&uuid, &data_str],
        )
        .context("Update task query")?;
        Ok(())
    }

    fn delete_task(&mut self, uuid: Uuid) -> anyhow::Result<bool> {
        let t = self.get_txn()?;
        let changed = t
            .execute("DELETE FROM tasks WHERE uuid = ?", [&uuid])
            .context("Delete task query")?;
        Ok(changed > 0)
    }

    fn all_tasks(&mut self) -> anyhow::Result<Vec<(Uuid, TaskMap)>> {
        let t = self.get_txn()?;

        let mut q = t.prepare("SELECT uuid, data FROM tasks")?;
        let rows = q.query_map([], |r| {
            let uuid: Uuid = r.get("uuid")?;
            let data_str: String = r.get("data")?;
            let data: TaskMap = serde_json::from_str(&data_str).unwrap(); // FIXME: Remove unwrap
            Ok((uuid, data))
        })?;

        let mut ret = vec![];
        for r in rows {
            ret.push(r?);
        }
        Ok(ret)
    }

    fn all_task_uuids(&mut self) -> anyhow::Result<Vec<Uuid>> {
        let t = self.get_txn()?;

        let mut q = t.prepare("SELECT uuid FROM tasks")?;
        let rows = q.query_map([], |r| {
            let uuid: Uuid = r.get("uuid")?;
            Ok(uuid)
        })?;

        let mut ret = vec![];
        for r in rows {
            ret.push(r?);
        }
        Ok(ret)
    }

    fn base_version(&mut self) -> anyhow::Result<VersionId> {
        let t = self.get_txn()?;

        let mut version = t
            .query_row(
                "SELECT value FROM sync_meta WHERE key = 'base_version'",
                [],
                |r| r.get("value"),
            )
            .optional()?;
        Ok(version.unwrap_or(DEFAULT_BASE_VERSION))
    }

    fn set_base_version(&mut self, version: VersionId) -> anyhow::Result<()> {
        let t = self.get_txn()?;
        t.execute(
            "INSERT OR REPLACE INTO sync_meta (key, value) VALUES (?, ?)",
            params!["base_version", &version],
        )
        .context("Set base version")?;
        Ok(())
    }

    fn operations(&mut self) -> anyhow::Result<Vec<Operation>> {
        let t = self.get_txn()?;

        let mut q = t.prepare("SELECT data FROM operations ORDER BY id ASC")?;
        let rows = q.query_map([], |r| {
            let data_str: String = r.get("data")?;
            let data: Operation = serde_json::from_str(&data_str).unwrap(); // FIXME: Remove unwrap
            Ok(data)
        })?;

        let mut ret = vec![];
        for r in rows {
            ret.push(r?);
        }
        Ok(ret)
    }

    fn add_operation(&mut self, op: Operation) -> anyhow::Result<()> {
        let t = self.get_txn()?;

        let data_str = serde_json::to_string(&op)?;
        t.execute(
            "INSERT INTO operations (data) VALUES (?)",
            params![&data_str],
        )
        .context("Add operation query")?;
        Ok(())
    }

    fn set_operations(&mut self, ops: Vec<Operation>) -> anyhow::Result<()> {
        let t = self.get_txn()?;
        t.execute(
            "DELETE FROM operations",
            [],
        )
        .context("Clear all existing operations")?;

        for o in ops {
            self.add_operation(o)?;
        }
        Ok(())
    }

    fn get_working_set(&mut self) -> anyhow::Result<Vec<Option<Uuid>>> {
        let t = self.get_txn()?;

        let mut q = t.prepare("SELECT id, uuid FROM working_set ORDER BY id ASC")?;
        let rows = q
            .query_map([], |r| {
                let id: usize = r.get("id")?;
                let uuid: Uuid = r.get("uuid")?;
                Ok((id, uuid))
            })
            .context("Get working set query")?;

        let rows: Vec<Result<(usize, Uuid), _>> = rows.collect();
        let mut res = Vec::with_capacity(rows.len());
        for _ in 0..self.get_next_working_set_number().context("HUh")? {
            res.push(None);
        }
        for r in rows {
            let (id, uuid) = r?;
            res[id as usize] = Some(uuid);
        }

        Ok(res)
    }

    fn add_to_working_set(&mut self, uuid: Uuid) -> anyhow::Result<usize> {
        let t = self.get_txn()?;

        let next_working_id = self.get_next_working_set_number()?;

        t.execute(
            "INSERT INTO working_set (id, uuid) VALUES (?, ?)",
            params![next_working_id, &uuid],
        )
        .context("Create task query")?;

        Ok(next_working_id)
    }

    fn set_working_set_item(&mut self, index: usize, uuid: Option<Uuid>) -> anyhow::Result<()> {
        let t = self.get_txn()?;
        match uuid {
            // Add or override item
            Some(uuid) => t.execute(
                "INSERT OR REPLACE INTO working_set (id, uuid) VALUES (?, ?)",
                params![index, &uuid],
            ),
            // Setting to None removes the row from database
            None => t.execute("DELETE FROM working_set WHERE id = ?", [index]),
        }
        .context("Set working set item query")?;
        Ok(())
    }

    fn clear_working_set(&mut self) -> anyhow::Result<()> {
        let t = self.get_txn()?;
        t.execute("DELETE FROM working_set", [])
            .context("Clear working set query")?;
        Ok(())
    }

    fn commit(&mut self) -> anyhow::Result<()> {
        let t = self
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
    use crate::storage::taskmap_with;
    use tempfile::TempDir;

    #[test]
    fn test_create() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid)?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(uuid)?;
            assert_eq!(task, Some(taskmap_with(vec![])));
        }
        Ok(())
    }

    #[test]
    fn test_create_exists() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid)?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert!(!txn.create_task(uuid)?);
            txn.commit()?;
        }
        Ok(())
    }

    #[test]
    fn test_get_missing() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(uuid)?;
            assert_eq!(task, None);
        }
        Ok(())
    }

    #[test]
    fn test_set_task() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            txn.set_task(uuid, taskmap_with(vec![("k".to_string(), "v".to_string())]))?;
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(uuid)?;
            assert_eq!(
                task,
                Some(taskmap_with(vec![("k".to_string(), "v".to_string())]))
            );
        }
        Ok(())
    }

    #[test]
    fn test_delete_task_missing() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(!txn.delete_task(uuid)?);
        }
        Ok(())
    }

    #[test]
    fn test_delete_task_exists() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid)?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert!(txn.delete_task(uuid)?);
        }
        Ok(())
    }

    #[test]
    fn test_all_tasks_empty() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        {
            let mut txn = storage.txn()?;
            let tasks = txn.all_tasks()?;
            assert_eq!(tasks, vec![]);
        }
        Ok(())
    }

    #[test]
    fn test_all_tasks_and_uuids() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid1.clone())?);
            txn.set_task(
                uuid1.clone(),
                taskmap_with(vec![("num".to_string(), "1".to_string())]),
            )?;
            assert!(txn.create_task(uuid2.clone())?);
            txn.set_task(
                uuid2.clone(),
                taskmap_with(vec![("num".to_string(), "2".to_string())]),
            )?;
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            let mut tasks = txn.all_tasks()?;

            // order is nondeterministic, so sort by uuid
            tasks.sort_by(|a, b| a.0.cmp(&b.0));

            let mut exp = vec![
                (
                    uuid1.clone(),
                    taskmap_with(vec![("num".to_string(), "1".to_string())]),
                ),
                (
                    uuid2.clone(),
                    taskmap_with(vec![("num".to_string(), "2".to_string())]),
                ),
            ];
            exp.sort_by(|a, b| a.0.cmp(&b.0));

            assert_eq!(tasks, exp);
        }
        {
            let mut txn = storage.txn()?;
            let mut uuids = txn.all_task_uuids()?;
            uuids.sort();

            let mut exp = vec![uuid1.clone(), uuid2.clone()];
            exp.sort();

            assert_eq!(uuids, exp);
        }
        Ok(())
    }

    #[test]
    fn test_base_version_default() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        {
            let mut txn = storage.txn()?;
            assert_eq!(txn.base_version()?, DEFAULT_BASE_VERSION);
        }
        Ok(())
    }

    #[test]
    fn test_base_version_setting() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let u = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            txn.set_base_version(u)?;
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert_eq!(txn.base_version()?, u);
        }
        Ok(())
    }

    #[test]
    fn test_operations() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        let uuid3 = Uuid::new_v4();

        // create some operations
        {
            let mut txn = storage.txn()?;
            txn.add_operation(Operation::Create { uuid: uuid1 })?;
            txn.add_operation(Operation::Create { uuid: uuid2 })?;
            txn.commit()?;
        }

        // read them back
        {
            let mut txn = storage.txn()?;
            let ops = txn.operations()?;
            assert_eq!(
                ops,
                vec![
                    Operation::Create { uuid: uuid1 },
                    Operation::Create { uuid: uuid2 },
                ]
            );
        }

        // set them to a different bunch
        {
            let mut txn = storage.txn()?;
            txn.set_operations(vec![
                Operation::Delete { uuid: uuid2 },
                Operation::Delete { uuid: uuid1 },
            ])?;
            txn.commit()?;
        }

        // create some more operations (to test adding operations after clearing)
        {
            let mut txn = storage.txn()?;
            txn.add_operation(Operation::Create { uuid: uuid3 })?;
            txn.add_operation(Operation::Delete { uuid: uuid3 })?;
            txn.commit()?;
        }

        // read them back
        {
            let mut txn = storage.txn()?;
            let ops = txn.operations()?;
            assert_eq!(
                ops,
                vec![
                    Operation::Delete { uuid: uuid2 },
                    Operation::Delete { uuid: uuid1 },
                    Operation::Create { uuid: uuid3 },
                    Operation::Delete { uuid: uuid3 },
                ]
            );
        }
        Ok(())
    }

    #[test]
    fn get_working_set_empty() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None]);
        }

        Ok(())
    }

    #[test]
    fn add_to_working_set() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1)?;
            txn.add_to_working_set(uuid2)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, Some(uuid1), Some(uuid2)]);
        }

        Ok(())
    }

    #[test]
    fn clear_working_set() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1)?;
            txn.add_to_working_set(uuid2)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            txn.clear_working_set()?;
            txn.add_to_working_set(uuid2)?;
            txn.add_to_working_set(uuid1)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, Some(uuid2), Some(uuid1)]);
        }

        Ok(())
    }

    #[test]
    fn set_working_set_item() -> anyhow::Result<()> {
        let tmp_dir = TempDir::new()?;
        let mut storage = SqliteStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1)?;
            txn.add_to_working_set(uuid2)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, Some(uuid1), Some(uuid2)]);
        }

        // Clear one item
        dbg!(1);
        {
            let mut txn = storage.txn()?;
            let ws = txn.set_working_set_item(1, None)?;
            txn.commit()?;
        }

        dbg!(2);
        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, None, Some(uuid2)]);
        }

        dbg!(3);
        // Override item
        {
            let mut txn = storage.txn()?;
            let ws = txn.set_working_set_item(2, Some(uuid1))?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, None, Some(uuid1)]);
        }

        Ok(())
    }
}

use crate::operation::Operation;
use crate::taskstorage::{TaskMap, TaskStorage, TaskStorageTxn};
use failure::Fallible;
use kv::msgpack::Msgpack;
use kv::{Bucket, Config, Error, Integer, Serde, Store, ValueBuf};
use std::convert::TryInto;
use std::path::Path;
use uuid::Uuid;

/// A representation of a UUID as a key.  This is just a newtype wrapping the 128-bit packed form
/// of a UUID.
#[derive(Debug, PartialEq, Eq, PartialOrd, Ord)]
struct Key(uuid::Bytes);

impl From<&[u8]> for Key {
    fn from(bytes: &[u8]) -> Key {
        let key = Key(bytes.try_into().unwrap());
        key
    }
}

impl From<&Uuid> for Key {
    fn from(uuid: &Uuid) -> Key {
        let key = Key(uuid.as_bytes().clone());
        key
    }
}

impl From<Uuid> for Key {
    fn from(uuid: Uuid) -> Key {
        let key = Key(uuid.as_bytes().clone());
        key
    }
}

impl From<Key> for Uuid {
    fn from(key: Key) -> Uuid {
        Uuid::from_bytes(key.0)
    }
}

impl AsRef<[u8]> for Key {
    fn as_ref(&self) -> &[u8] {
        &self.0[..]
    }
}

/// KVStorage is an on-disk storage backend which uses LMDB via the `kv` crate.
pub struct KVStorage<'t> {
    store: Store,
    tasks_bucket: Bucket<'t, Key, ValueBuf<Msgpack<TaskMap>>>,
    numbers_bucket: Bucket<'t, Integer, ValueBuf<Msgpack<u64>>>,
    operations_bucket: Bucket<'t, Integer, ValueBuf<Msgpack<Operation>>>,
    working_set_bucket: Bucket<'t, Integer, ValueBuf<Msgpack<Uuid>>>,
}

const BASE_VERSION: u64 = 1;
const NEXT_OPERATION: u64 = 2;
const NEXT_WORKING_SET_INDEX: u64 = 3;

impl<'t> KVStorage<'t> {
    pub fn new(directory: &Path) -> Fallible<KVStorage> {
        let mut config = Config::default(directory);
        config.bucket("tasks", None);
        config.bucket("numbers", None);
        config.bucket("operations", None);
        config.bucket("working_set", None);
        let store = Store::new(config)?;

        // tasks are stored indexed by uuid
        let tasks_bucket = store.bucket::<Key, ValueBuf<Msgpack<TaskMap>>>(Some("tasks"))?;

        // this bucket contains various u64s, indexed by constants above
        let numbers_bucket = store.int_bucket::<ValueBuf<Msgpack<u64>>>(Some("numbers"))?;

        // this bucket contains operations, numbered consecutively; the NEXT_OPERATION number gives
        // the index of the next operation to insert
        let operations_bucket =
            store.int_bucket::<ValueBuf<Msgpack<Operation>>>(Some("operations"))?;

        // this bucket contains operations, numbered consecutively; the NEXT_WORKING_SET_INDEX
        // number gives the index of the next operation to insert
        let working_set_bucket =
            store.int_bucket::<ValueBuf<Msgpack<Uuid>>>(Some("working_set"))?;

        Ok(KVStorage {
            store,
            tasks_bucket,
            numbers_bucket,
            operations_bucket,
            working_set_bucket,
        })
    }
}

impl<'t> TaskStorage for KVStorage<'t> {
    fn txn<'a>(&'a mut self) -> Fallible<Box<dyn TaskStorageTxn + 'a>> {
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
    fn kvtxn<'a>(&mut self) -> &mut kv::Txn<'t> {
        if let Some(ref mut txn) = self.txn {
            txn
        } else {
            panic!("cannot use transaction after commit");
        }
    }

    // Access to buckets
    fn tasks_bucket(&self) -> &'t Bucket<'t, Key, ValueBuf<Msgpack<TaskMap>>> {
        &self.storage.tasks_bucket
    }
    fn numbers_bucket(&self) -> &'t Bucket<'t, Integer, ValueBuf<Msgpack<u64>>> {
        &self.storage.numbers_bucket
    }
    fn operations_bucket(&self) -> &'t Bucket<'t, Integer, ValueBuf<Msgpack<Operation>>> {
        &self.storage.operations_bucket
    }
    fn working_set_bucket(&self) -> &'t Bucket<'t, Integer, ValueBuf<Msgpack<Uuid>>> {
        &self.storage.working_set_bucket
    }
}

impl<'t> TaskStorageTxn for Txn<'t> {
    fn get_task(&mut self, uuid: &Uuid) -> Fallible<Option<TaskMap>> {
        let bucket = self.tasks_bucket();
        let buf = match self.kvtxn().get(bucket, uuid.into()) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(None),
            Err(e) => return Err(e.into()),
        };
        let value = buf.inner()?.to_serde();
        Ok(Some(value))
    }

    fn create_task(&mut self, uuid: Uuid) -> Fallible<bool> {
        let bucket = self.tasks_bucket();
        let kvtxn = self.kvtxn();
        match kvtxn.get(bucket, uuid.into()) {
            Err(Error::NotFound) => {
                kvtxn.set(bucket, uuid.into(), Msgpack::to_value_buf(TaskMap::new())?)?;
                Ok(true)
            }
            Err(e) => Err(e.into()),
            Ok(_) => Ok(false),
        }
    }

    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> Fallible<()> {
        let bucket = self.tasks_bucket();
        let kvtxn = self.kvtxn();
        kvtxn.set(bucket, uuid.into(), Msgpack::to_value_buf(task)?)?;
        Ok(())
    }

    fn delete_task(&mut self, uuid: &Uuid) -> Fallible<bool> {
        let bucket = self.tasks_bucket();
        let kvtxn = self.kvtxn();
        match kvtxn.del(bucket, uuid.into()) {
            Err(Error::NotFound) => Ok(false),
            Err(e) => Err(e.into()),
            Ok(_) => Ok(true),
        }
    }

    fn all_tasks(&mut self) -> Fallible<Vec<(Uuid, TaskMap)>> {
        let bucket = self.tasks_bucket();
        let kvtxn = self.kvtxn();
        let all_tasks: Result<Vec<(Uuid, TaskMap)>, Error> = kvtxn
            .read_cursor(bucket)?
            .iter()
            .map(|(k, v)| Ok((k.into(), v.inner()?.to_serde())))
            .collect();
        Ok(all_tasks?)
    }

    fn all_task_uuids(&mut self) -> Fallible<Vec<Uuid>> {
        let bucket = self.tasks_bucket();
        let kvtxn = self.kvtxn();
        Ok(kvtxn
            .read_cursor(bucket)?
            .iter()
            .map(|(k, _)| k.into())
            .collect())
    }

    fn base_version(&mut self) -> Fallible<u64> {
        let bucket = self.numbers_bucket();
        let base_version = match self.kvtxn().get(bucket, BASE_VERSION.into()) {
            Ok(buf) => buf,
            Err(Error::NotFound) => return Ok(0),
            Err(e) => return Err(e.into()),
        }
        .inner()?
        .to_serde();
        Ok(base_version)
    }

    fn set_base_version(&mut self, version: u64) -> Fallible<()> {
        let numbers_bucket = self.numbers_bucket();
        let kvtxn = self.kvtxn();

        kvtxn.set(
            numbers_bucket,
            BASE_VERSION.into(),
            Msgpack::to_value_buf(version)?,
        )?;
        Ok(())
    }

    fn operations(&mut self) -> Fallible<Vec<Operation>> {
        let bucket = self.operations_bucket();
        let kvtxn = self.kvtxn();
        let all_ops: Result<Vec<(u64, Operation)>, Error> = kvtxn
            .read_cursor(bucket)?
            .iter()
            .map(|(i, v)| Ok((i.into(), v.inner()?.to_serde())))
            .collect();
        let mut all_ops = all_ops?;
        // sort by key..
        all_ops.sort_by(|a, b| a.0.cmp(&b.0));
        // and return the values..
        Ok(all_ops.iter().map(|(_, v)| v.clone()).collect())
    }

    fn add_operation(&mut self, op: Operation) -> Fallible<()> {
        let numbers_bucket = self.numbers_bucket();
        let operations_bucket = self.operations_bucket();
        let kvtxn = self.kvtxn();

        let next_op = match kvtxn.get(numbers_bucket, NEXT_OPERATION.into()) {
            Ok(buf) => buf.inner()?.to_serde(),
            Err(Error::NotFound) => 0,
            Err(e) => return Err(e.into()),
        };

        kvtxn.set(
            operations_bucket,
            next_op.into(),
            Msgpack::to_value_buf(op)?,
        )?;
        kvtxn.set(
            numbers_bucket,
            NEXT_OPERATION.into(),
            Msgpack::to_value_buf(next_op + 1)?,
        )?;
        Ok(())
    }

    fn set_operations(&mut self, ops: Vec<Operation>) -> Fallible<()> {
        let numbers_bucket = self.numbers_bucket();
        let operations_bucket = self.operations_bucket();
        let kvtxn = self.kvtxn();

        kvtxn.clear_db(operations_bucket)?;

        let mut i = 0u64;
        for op in ops {
            kvtxn.set(operations_bucket, i.into(), Msgpack::to_value_buf(op)?)?;
            i += 1;
        }

        kvtxn.set(
            numbers_bucket,
            NEXT_OPERATION.into(),
            Msgpack::to_value_buf(i)?,
        )?;

        Ok(())
    }

    fn get_working_set(&mut self) -> Fallible<Vec<Option<Uuid>>> {
        let working_set_bucket = self.working_set_bucket();
        let numbers_bucket = self.numbers_bucket();
        let kvtxn = self.kvtxn();

        let next_index = match kvtxn.get(numbers_bucket, NEXT_WORKING_SET_INDEX.into()) {
            Ok(buf) => buf.inner()?.to_serde(),
            Err(Error::NotFound) => 1,
            Err(e) => return Err(e.into()),
        };

        let mut res = Vec::with_capacity(next_index as usize);
        for _ in 0..next_index {
            res.push(None)
        }

        for (i, u) in kvtxn.read_cursor(working_set_bucket)?.iter() {
            let i: u64 = i.into();
            res[i as usize] = Some(u.inner()?.to_serde());
        }
        Ok(res)
    }

    fn add_to_working_set(&mut self, uuid: &Uuid) -> Fallible<u64> {
        let working_set_bucket = self.working_set_bucket();
        let numbers_bucket = self.numbers_bucket();
        let kvtxn = self.kvtxn();

        let next_index = match kvtxn.get(numbers_bucket, NEXT_WORKING_SET_INDEX.into()) {
            Ok(buf) => buf.inner()?.to_serde(),
            Err(Error::NotFound) => 1,
            Err(e) => return Err(e.into()),
        };

        kvtxn.set(
            working_set_bucket,
            next_index.into(),
            Msgpack::to_value_buf(uuid.clone())?,
        )?;
        kvtxn.set(
            numbers_bucket,
            NEXT_WORKING_SET_INDEX.into(),
            Msgpack::to_value_buf(next_index + 1)?,
        )?;
        Ok(next_index)
    }

    fn remove_from_working_set(&mut self, index: u64) -> Fallible<()> {
        let working_set_bucket = self.working_set_bucket();
        let numbers_bucket = self.numbers_bucket();
        let kvtxn = self.kvtxn();

        let next_index = match kvtxn.get(numbers_bucket, NEXT_WORKING_SET_INDEX.into()) {
            Ok(buf) => buf.inner()?.to_serde(),
            Err(Error::NotFound) => 1,
            Err(e) => return Err(e.into()),
        };
        if index == 0 || index >= next_index {
            return Err(format_err!("No task found with index {}", index));
        }

        match kvtxn.del(working_set_bucket, index.into()) {
            Err(Error::NotFound) => Err(format_err!("No task found with index {}", index)),
            Err(e) => Err(e.into()),
            Ok(_) => Ok(()),
        }
    }

    fn clear_working_set(&mut self) -> Fallible<()> {
        let working_set_bucket = self.working_set_bucket();
        let numbers_bucket = self.numbers_bucket();
        let kvtxn = self.kvtxn();

        kvtxn.clear_db(working_set_bucket)?;
        kvtxn.set(
            numbers_bucket,
            NEXT_WORKING_SET_INDEX.into(),
            Msgpack::to_value_buf(1)?,
        )?;

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
    use crate::taskstorage::taskmap_with;
    use failure::Fallible;
    use tempdir::TempDir;

    #[test]
    fn test_create() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid.clone())?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(&uuid)?;
            assert_eq!(task, Some(taskmap_with(vec![])));
        }
        Ok(())
    }

    #[test]
    fn test_create_exists() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid.clone())?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert!(!txn.create_task(uuid.clone())?);
            txn.commit()?;
        }
        Ok(())
    }

    #[test]
    fn test_get_missing() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(&uuid)?;
            assert_eq!(task, None);
        }
        Ok(())
    }

    #[test]
    fn test_set_task() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            txn.set_task(
                uuid.clone(),
                taskmap_with(vec![("k".to_string(), "v".to_string())]),
            )?;
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            let task = txn.get_task(&uuid)?;
            assert_eq!(
                task,
                Some(taskmap_with(vec![("k".to_string(), "v".to_string())]))
            );
        }
        Ok(())
    }

    #[test]
    fn test_delete_task_missing() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(!txn.delete_task(&uuid)?);
        }
        Ok(())
    }

    #[test]
    fn test_delete_task_exists() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid = Uuid::new_v4();
        {
            let mut txn = storage.txn()?;
            assert!(txn.create_task(uuid.clone())?);
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert!(txn.delete_task(&uuid)?);
        }
        Ok(())
    }

    #[test]
    fn test_all_tasks_empty() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        {
            let mut txn = storage.txn()?;
            let tasks = txn.all_tasks()?;
            assert_eq!(tasks, vec![]);
        }
        Ok(())
    }

    #[test]
    fn test_all_tasks_and_uuids() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
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
    fn test_base_version_default() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        {
            let mut txn = storage.txn()?;
            assert_eq!(txn.base_version()?, 0);
        }
        Ok(())
    }

    #[test]
    fn test_base_version_setting() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        {
            let mut txn = storage.txn()?;
            txn.set_base_version(3)?;
            txn.commit()?;
        }
        {
            let mut txn = storage.txn()?;
            assert_eq!(txn.base_version()?, 3);
        }
        Ok(())
    }

    #[test]
    fn test_operations() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
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
    fn get_working_set_empty() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None]);
        }

        Ok(())
    }

    #[test]
    fn add_to_working_set() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(&uuid1)?;
            txn.add_to_working_set(&uuid2)?;
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
    fn add_and_remove_from_working_set_holes() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(&uuid1)?;
            txn.add_to_working_set(&uuid2)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            txn.remove_from_working_set(1)?;
            txn.add_to_working_set(&uuid1)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, None, Some(uuid2), Some(uuid1)]);
        }

        Ok(())
    }

    #[test]
    fn remove_working_set_doesnt_exist() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(&uuid1)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let res = txn.remove_from_working_set(0);
            assert!(res.is_err());
            let res = txn.remove_from_working_set(2);
            assert!(res.is_err());
        }

        Ok(())
    }

    #[test]
    fn clear_working_set() -> Fallible<()> {
        let tmp_dir = TempDir::new("test")?;
        let mut storage = KVStorage::new(&tmp_dir.path())?;
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(&uuid1)?;
            txn.add_to_working_set(&uuid2)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            txn.clear_working_set()?;
            txn.add_to_working_set(&uuid2)?;
            txn.add_to_working_set(&uuid1)?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None, Some(uuid2), Some(uuid1)]);
        }

        Ok(())
    }
}

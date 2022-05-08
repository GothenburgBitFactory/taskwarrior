#![allow(clippy::new_without_default)]

use crate::storage::{ReplicaOp, Storage, StorageTxn, TaskMap, VersionId, DEFAULT_BASE_VERSION};
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use uuid::Uuid;

#[derive(PartialEq, Debug, Clone)]
struct Data {
    tasks: HashMap<Uuid, TaskMap>,
    base_version: VersionId,
    operations: Vec<ReplicaOp>,
    working_set: Vec<Option<Uuid>>,
}

struct Txn<'t> {
    storage: &'t mut InMemoryStorage,
    new_data: Option<Data>,
}

impl<'t> Txn<'t> {
    fn mut_data_ref(&mut self) -> &mut Data {
        if self.new_data.is_none() {
            self.new_data = Some(self.storage.data.clone());
        }
        if let Some(ref mut data) = self.new_data {
            data
        } else {
            unreachable!();
        }
    }

    fn data_ref(&mut self) -> &Data {
        if let Some(ref data) = self.new_data {
            data
        } else {
            &self.storage.data
        }
    }
}

impl<'t> StorageTxn for Txn<'t> {
    fn get_task(&mut self, uuid: Uuid) -> anyhow::Result<Option<TaskMap>> {
        match self.data_ref().tasks.get(&uuid) {
            None => Ok(None),
            Some(t) => Ok(Some(t.clone())),
        }
    }

    fn create_task(&mut self, uuid: Uuid) -> anyhow::Result<bool> {
        if let ent @ Entry::Vacant(_) = self.mut_data_ref().tasks.entry(uuid) {
            ent.or_insert_with(TaskMap::new);
            Ok(true)
        } else {
            Ok(false)
        }
    }

    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> anyhow::Result<()> {
        self.mut_data_ref().tasks.insert(uuid, task);
        Ok(())
    }

    fn delete_task(&mut self, uuid: Uuid) -> anyhow::Result<bool> {
        Ok(self.mut_data_ref().tasks.remove(&uuid).is_some())
    }

    fn all_tasks<'a>(&mut self) -> anyhow::Result<Vec<(Uuid, TaskMap)>> {
        Ok(self
            .data_ref()
            .tasks
            .iter()
            .map(|(u, t)| (*u, t.clone()))
            .collect())
    }

    fn all_task_uuids<'a>(&mut self) -> anyhow::Result<Vec<Uuid>> {
        Ok(self.data_ref().tasks.keys().copied().collect())
    }

    fn base_version(&mut self) -> anyhow::Result<VersionId> {
        Ok(self.data_ref().base_version)
    }

    fn set_base_version(&mut self, version: VersionId) -> anyhow::Result<()> {
        self.mut_data_ref().base_version = version;
        Ok(())
    }

    fn operations(&mut self) -> anyhow::Result<Vec<ReplicaOp>> {
        Ok(self.data_ref().operations.clone())
    }

    fn num_operations(&mut self) -> anyhow::Result<usize> {
        Ok(self.data_ref().operations.len())
    }

    fn add_operation(&mut self, op: ReplicaOp) -> anyhow::Result<()> {
        self.mut_data_ref().operations.push(op);
        Ok(())
    }

    fn set_operations(&mut self, ops: Vec<ReplicaOp>) -> anyhow::Result<()> {
        self.mut_data_ref().operations = ops;
        Ok(())
    }

    fn get_working_set(&mut self) -> anyhow::Result<Vec<Option<Uuid>>> {
        Ok(self.data_ref().working_set.clone())
    }

    fn add_to_working_set(&mut self, uuid: Uuid) -> anyhow::Result<usize> {
        let working_set = &mut self.mut_data_ref().working_set;
        working_set.push(Some(uuid));
        Ok(working_set.len())
    }

    fn set_working_set_item(&mut self, index: usize, uuid: Option<Uuid>) -> anyhow::Result<()> {
        let working_set = &mut self.mut_data_ref().working_set;
        if index >= working_set.len() {
            anyhow::bail!("Index {} is not in the working set", index);
        }
        working_set[index] = uuid;
        Ok(())
    }

    fn clear_working_set(&mut self) -> anyhow::Result<()> {
        self.mut_data_ref().working_set = vec![None];
        Ok(())
    }

    fn commit(&mut self) -> anyhow::Result<()> {
        // copy the new_data back into storage to commit the transaction
        if let Some(data) = self.new_data.take() {
            self.storage.data = data;
        }
        Ok(())
    }
}

/// InMemoryStorage is a simple in-memory task storage implementation.  It is not useful for
/// production data, but is useful for testing purposes.
#[derive(PartialEq, Debug, Clone)]
pub struct InMemoryStorage {
    data: Data,
}

impl InMemoryStorage {
    pub fn new() -> InMemoryStorage {
        InMemoryStorage {
            data: Data {
                tasks: HashMap::new(),
                base_version: DEFAULT_BASE_VERSION,
                operations: vec![],
                working_set: vec![None],
            },
        }
    }
}

impl Storage for InMemoryStorage {
    fn txn<'a>(&'a mut self) -> anyhow::Result<Box<dyn StorageTxn + 'a>> {
        Ok(Box::new(Txn {
            storage: self,
            new_data: None,
        }))
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    // (note: this module is heavily used in tests so most of its functionality is well-tested
    // elsewhere and not tested here)

    #[test]
    fn get_working_set_empty() -> anyhow::Result<()> {
        let mut storage = InMemoryStorage::new();

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None]);
        }

        Ok(())
    }

    #[test]
    fn add_to_working_set() -> anyhow::Result<()> {
        let mut storage = InMemoryStorage::new();
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
        let mut storage = InMemoryStorage::new();
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
}

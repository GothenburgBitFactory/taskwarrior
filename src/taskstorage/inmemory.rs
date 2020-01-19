use crate::operation::Operation;
use crate::taskstorage::{TaskMap, TaskStorage, TaskStorageTxn};
use failure::Fallible;
use std::collections::hash_map::Entry;
use std::collections::HashMap;
use uuid::Uuid;

#[derive(PartialEq, Debug, Clone)]
struct Data {
    tasks: HashMap<Uuid, TaskMap>,
    base_version: u64,
    operations: Vec<Operation>,
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

impl<'t> TaskStorageTxn for Txn<'t> {
    fn get_task(&mut self, uuid: &Uuid) -> Fallible<Option<TaskMap>> {
        match self.data_ref().tasks.get(uuid) {
            None => Ok(None),
            Some(t) => Ok(Some(t.clone())),
        }
    }

    fn create_task(&mut self, uuid: Uuid) -> Fallible<bool> {
        if let ent @ Entry::Vacant(_) = self.mut_data_ref().tasks.entry(uuid) {
            ent.or_insert(TaskMap::new());
            Ok(true)
        } else {
            Ok(false)
        }
    }

    fn set_task(&mut self, uuid: Uuid, task: TaskMap) -> Fallible<()> {
        self.mut_data_ref().tasks.insert(uuid, task);
        Ok(())
    }

    fn delete_task(&mut self, uuid: &Uuid) -> Fallible<bool> {
        if let Some(_) = self.mut_data_ref().tasks.remove(uuid) {
            Ok(true)
        } else {
            Ok(false)
        }
    }

    fn all_tasks<'a>(&mut self) -> Fallible<Vec<(Uuid, TaskMap)>> {
        Ok(self
            .data_ref()
            .tasks
            .iter()
            .map(|(u, t)| (u.clone(), t.clone()))
            .collect())
    }

    fn all_task_uuids<'a>(&mut self) -> Fallible<Vec<Uuid>> {
        Ok(self.data_ref().tasks.keys().map(|u| u.clone()).collect())
    }

    fn base_version(&mut self) -> Fallible<u64> {
        Ok(self.data_ref().base_version)
    }

    fn set_base_version(&mut self, version: u64) -> Fallible<()> {
        self.mut_data_ref().base_version = version;
        Ok(())
    }

    fn operations(&mut self) -> Fallible<Vec<Operation>> {
        Ok(self.data_ref().operations.clone())
    }

    fn add_operation(&mut self, op: Operation) -> Fallible<()> {
        self.mut_data_ref().operations.push(op);
        Ok(())
    }

    fn set_operations(&mut self, ops: Vec<Operation>) -> Fallible<()> {
        self.mut_data_ref().operations = ops;
        Ok(())
    }

    fn get_working_set(&mut self) -> Fallible<Vec<Option<Uuid>>> {
        Ok(self.data_ref().working_set.clone())
    }

    fn add_to_working_set(&mut self, uuid: Uuid) -> Fallible<u64> {
        let working_set = &mut self.mut_data_ref().working_set;
        working_set.push(Some(uuid));
        Ok(working_set.len() as u64)
    }

    fn remove_from_working_set(&mut self, index: u64) -> Fallible<()> {
        let index = index as usize;
        let working_set = &mut self.mut_data_ref().working_set;
        if index >= working_set.len() || working_set[index].is_none() {
            return Err(format_err!("No task found with index {}", index));
        }
        working_set[index] = None;
        Ok(())
    }

    fn clear_working_set(&mut self) -> Fallible<()> {
        self.mut_data_ref().working_set = vec![None];
        Ok(())
    }

    fn commit(&mut self) -> Fallible<()> {
        // copy the new_data back into storage to commit the transaction
        if let Some(data) = self.new_data.take() {
            self.storage.data = data;
        }
        Ok(())
    }
}

#[derive(PartialEq, Debug, Clone)]
pub struct InMemoryStorage {
    data: Data,
}

impl InMemoryStorage {
    pub fn new() -> InMemoryStorage {
        InMemoryStorage {
            data: Data {
                tasks: HashMap::new(),
                base_version: 0,
                operations: vec![],
                working_set: vec![None],
            },
        }
    }
}

impl TaskStorage for InMemoryStorage {
    fn txn<'a>(&'a mut self) -> Fallible<Box<dyn TaskStorageTxn + 'a>> {
        Ok(Box::new(Txn {
            storage: self,
            new_data: None,
        }))
    }
}

#[cfg(test)]
mod test {
    use super::*;

    // (note: this module is heavily used in tests so most of its functionality is well-tested
    // elsewhere and not tested here)

    #[test]
    fn get_working_set_empty() -> Fallible<()> {
        let mut storage = InMemoryStorage::new();

        {
            let mut txn = storage.txn()?;
            let ws = txn.get_working_set()?;
            assert_eq!(ws, vec![None]);
        }

        Ok(())
    }

    #[test]
    fn add_to_working_set() -> Fallible<()> {
        let mut storage = InMemoryStorage::new();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1.clone())?;
            txn.add_to_working_set(uuid2.clone())?;
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
        let mut storage = InMemoryStorage::new();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1.clone())?;
            txn.add_to_working_set(uuid2.clone())?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            txn.remove_from_working_set(1)?;
            txn.add_to_working_set(uuid1.clone())?;
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
        let mut storage = InMemoryStorage::new();
        let uuid1 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1.clone())?;
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
        let mut storage = InMemoryStorage::new();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        {
            let mut txn = storage.txn()?;
            txn.add_to_working_set(uuid1.clone())?;
            txn.add_to_working_set(uuid2.clone())?;
            txn.commit()?;
        }

        {
            let mut txn = storage.txn()?;
            txn.clear_working_set()?;
            txn.add_to_working_set(uuid2.clone())?;
            txn.add_to_working_set(uuid1.clone())?;
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

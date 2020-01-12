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

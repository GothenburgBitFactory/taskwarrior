use crate::errors::Error;
use crate::storage::{Operation, StorageTxn};

pub(super) fn apply_op(txn: &mut dyn StorageTxn, op: &Operation) -> anyhow::Result<()> {
    match op {
        Operation::Create { uuid } => {
            // insert if the task does not already exist
            if !txn.create_task(*uuid)? {
                return Err(Error::Database(format!("Task {} already exists", uuid)).into());
            }
        }
        Operation::Delete { ref uuid } => {
            if !txn.delete_task(*uuid)? {
                return Err(Error::Database(format!("Task {} does not exist", uuid)).into());
            }
        }
        Operation::Update {
            ref uuid,
            ref property,
            ref value,
            timestamp: _,
        } => {
            // update if this task exists, otherwise ignore
            if let Some(mut task) = txn.get_task(*uuid)? {
                match value {
                    Some(ref val) => task.insert(property.to_string(), val.clone()),
                    None => task.remove(property),
                };
                txn.set_task(*uuid, task)?;
            } else {
                return Err(Error::Database(format!("Task {} does not exist", uuid)).into());
            }
        }
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::taskdb::TaskDb;
    use chrono::Utc;
    use pretty_assertions::assert_eq;
    use std::collections::HashMap;
    use uuid::Uuid;

    #[test]
    fn test_apply_create() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };

        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op)?;
            txn.commit()?;
        }

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![]),]);
        Ok(())
    }

    #[test]
    fn test_apply_create_exists() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Create { uuid };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op)?;
            assert_eq!(
                apply_op(txn.as_mut(), &op).err().unwrap().to_string(),
                format!("Task Database Error: Task {} already exists", uuid)
            );
            txn.commit()?;
        }

        // first op was applied
        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![])]);

        Ok(())
    }

    #[test]
    fn test_apply_create_update() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };

        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op1)?;
            txn.commit()?;
        }

        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op2)?;
            txn.commit()?;
        }

        assert_eq!(
            db.sorted_tasks(),
            vec![(uuid, vec![("title".into(), "my task".into())])]
        );

        Ok(())
    }

    #[test]
    fn test_apply_create_update_delete_prop() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op1)?;
            txn.commit()?;
        }

        let op2 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op2)?;
            txn.commit()?;
        }

        let op3 = Operation::Update {
            uuid,
            property: String::from("priority"),
            value: Some("H".into()),
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op3)?;
            txn.commit()?;
        }

        let op4 = Operation::Update {
            uuid,
            property: String::from("title"),
            value: None,
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op4)?;
            txn.commit()?;
        }

        let mut exp = HashMap::new();
        let mut task = HashMap::new();
        task.insert(String::from("priority"), String::from("H"));
        exp.insert(uuid, task);
        assert_eq!(
            db.sorted_tasks(),
            vec![(uuid, vec![("priority".into(), "H".into())])]
        );

        Ok(())
    }

    #[test]
    fn test_apply_update_does_not_exist() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            assert_eq!(
                apply_op(txn.as_mut(), &op).err().unwrap().to_string(),
                format!("Task Database Error: Task {} does not exist", uuid)
            );
            txn.commit()?;
        }

        Ok(())
    }

    #[test]
    fn test_apply_create_delete() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op1 = Operation::Create { uuid };
        let op2 = Operation::Delete { uuid };

        {
            let mut txn = db.storage.txn()?;
            apply_op(txn.as_mut(), &op1)?;
            apply_op(txn.as_mut(), &op2)?;
            txn.commit()?;
        }

        assert_eq!(db.sorted_tasks(), vec![]);

        Ok(())
    }

    #[test]
    fn test_apply_delete_not_present() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = Operation::Delete { uuid };
        {
            let mut txn = db.storage.txn()?;
            assert_eq!(
                apply_op(txn.as_mut(), &op).err().unwrap().to_string(),
                format!("Task Database Error: Task {} does not exist", uuid)
            );
            txn.commit()?;
        }

        Ok(())
    }
}

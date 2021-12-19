use crate::errors::Error;
use crate::server::SyncOp;
use crate::storage::{ReplicaOp, StorageTxn, TaskMap};

/// Apply the given SyncOp to the replica, updating both the task data and adding a
/// ReplicaOp to the list of operations.  Returns the TaskMap of the task after the
/// operation has been applied (or an empty TaskMap for Delete).
pub(super) fn apply(txn: &mut dyn StorageTxn, op: SyncOp) -> anyhow::Result<TaskMap> {
    match op {
        SyncOp::Create { uuid } => {
            let created = txn.create_task(uuid)?;
            if created {
                txn.add_operation(ReplicaOp::Create { uuid })?;
                txn.commit()?;
                Ok(TaskMap::new())
            } else {
                // TODO: differentiate error types here?
                Err(Error::Database(format!("Task {} already exists", uuid)).into())
            }
        }
        SyncOp::Delete { uuid } => {
            let task = txn.get_task(uuid)?;
            // (we'll need _task in the next commit)
            if let Some(_task) = task {
                txn.delete_task(uuid)?;
                txn.add_operation(ReplicaOp::Delete { uuid })?;
                txn.commit()?;
                Ok(TaskMap::new())
            } else {
                Err(Error::Database(format!("Task {} does not exist", uuid)).into())
            }
        }
        SyncOp::Update {
            uuid,
            property,
            value,
            timestamp,
        } => {
            let task = txn.get_task(uuid)?;
            if let Some(mut task) = task {
                if let Some(ref v) = value {
                    task.insert(property.clone(), v.clone());
                } else {
                    task.remove(&property);
                }
                txn.set_task(uuid, task.clone())?;
                txn.add_operation(ReplicaOp::Update {
                    uuid,
                    property,
                    value,
                    timestamp,
                })?;
                txn.commit()?;
                Ok(task)
            } else {
                Err(Error::Database(format!("Task {} does not exist", uuid)).into())
            }
        }
    }
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
        let op = SyncOp::Create { uuid };

        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op)?;
            assert_eq!(taskmap.len(), 0);
            txn.commit()?;
        }

        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![]),]);
        assert_eq!(db.operations(), vec![ReplicaOp::Create { uuid }]);
        Ok(())
    }

    #[test]
    fn test_apply_create_exists() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = SyncOp::Create { uuid };
        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op.clone())?;
            assert_eq!(taskmap.len(), 0);
            assert_eq!(
                apply(txn.as_mut(), op).err().unwrap().to_string(),
                format!("Task Database Error: Task {} already exists", uuid)
            );
            txn.commit()?;
        }

        // first op was applied
        assert_eq!(db.sorted_tasks(), vec![(uuid, vec![])]);
        assert_eq!(db.operations(), vec![ReplicaOp::Create { uuid }]);
        Ok(())
    }

    #[test]
    fn test_apply_create_update() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let now = Utc::now();
        let op1 = SyncOp::Create { uuid };

        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op1)?;
            assert_eq!(taskmap.len(), 0);
            txn.commit()?;
        }

        let op2 = SyncOp::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: now,
        };
        {
            let mut txn = db.storage.txn()?;
            let mut taskmap = apply(txn.as_mut(), op2)?;
            assert_eq!(
                taskmap.drain().collect::<Vec<(_, _)>>(),
                vec![("title".into(), "my task".into())]
            );
            txn.commit()?;
        }

        assert_eq!(
            db.sorted_tasks(),
            vec![(uuid, vec![("title".into(), "my task".into())])]
        );
        assert_eq!(
            db.operations(),
            vec![
                ReplicaOp::Create { uuid },
                ReplicaOp::Update {
                    uuid,
                    property: "title".into(),
                    value: Some("my task".into()),
                    timestamp: now
                }
            ]
        );

        Ok(())
    }

    #[test]
    fn test_apply_create_update_delete_prop() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let now = Utc::now();
        let op1 = SyncOp::Create { uuid };
        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op1)?;
            assert_eq!(taskmap.len(), 0);
            txn.commit()?;
        }

        let op2 = SyncOp::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: now,
        };
        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op2)?;
            assert_eq!(taskmap.get("title"), Some(&"my task".to_owned()));
            txn.commit()?;
        }

        let op3 = SyncOp::Update {
            uuid,
            property: String::from("priority"),
            value: Some("H".into()),
            timestamp: now,
        };
        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op3)?;
            assert_eq!(taskmap.get("priority"), Some(&"H".to_owned()));
            txn.commit()?;
        }

        let op4 = SyncOp::Update {
            uuid,
            property: String::from("title"),
            value: None,
            timestamp: now,
        };
        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op4)?;
            assert_eq!(taskmap.get("title"), None);
            assert_eq!(taskmap.get("priority"), Some(&"H".to_owned()));
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
        assert_eq!(
            db.operations(),
            vec![
                ReplicaOp::Create { uuid },
                ReplicaOp::Update {
                    uuid,
                    property: "title".into(),
                    value: Some("my task".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid,
                    property: "priority".into(),
                    value: Some("H".into()),
                    timestamp: now,
                },
                ReplicaOp::Update {
                    uuid,
                    property: "title".into(),
                    value: None,
                    timestamp: now,
                }
            ]
        );

        Ok(())
    }

    #[test]
    fn test_apply_update_does_not_exist() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = SyncOp::Update {
            uuid,
            property: String::from("title"),
            value: Some("my task".into()),
            timestamp: Utc::now(),
        };
        {
            let mut txn = db.storage.txn()?;
            assert_eq!(
                apply(txn.as_mut(), op).err().unwrap().to_string(),
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
        let op1 = SyncOp::Create { uuid };
        let op2 = SyncOp::Delete { uuid };

        {
            let mut txn = db.storage.txn()?;
            let taskmap = apply(txn.as_mut(), op1)?;
            assert_eq!(taskmap.len(), 0);
            let taskmap = apply(txn.as_mut(), op2)?;
            assert_eq!(taskmap.len(), 0);
            txn.commit()?;
        }

        assert_eq!(db.sorted_tasks(), vec![]);
        assert_eq!(
            db.operations(),
            vec![ReplicaOp::Create { uuid }, ReplicaOp::Delete { uuid },]
        );

        Ok(())
    }

    #[test]
    fn test_apply_delete_not_present() -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid = Uuid::new_v4();
        let op = SyncOp::Delete { uuid };
        {
            let mut txn = db.storage.txn()?;
            assert_eq!(
                apply(txn.as_mut(), op).err().unwrap().to_string(),
                format!("Task Database Error: Task {} does not exist", uuid)
            );
            txn.commit()?;
        }

        Ok(())
    }
}

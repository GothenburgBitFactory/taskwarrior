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

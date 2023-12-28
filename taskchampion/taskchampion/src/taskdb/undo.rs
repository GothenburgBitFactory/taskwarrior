use super::apply;
use crate::errors::Result;
use crate::storage::{ReplicaOp, StorageTxn};
use log::{debug, info, trace};

/// Local operations until the most recent UndoPoint.
pub fn get_undo_ops(txn: &mut dyn StorageTxn) -> Result<Vec<ReplicaOp>> {
    let mut local_ops = txn.operations().unwrap();
    let mut undo_ops: Vec<ReplicaOp> = Vec::new();

    while let Some(op) = local_ops.pop() {
        if op == ReplicaOp::UndoPoint {
            break;
        }
        undo_ops.push(op);
    }

    Ok(undo_ops)
}

/// Commit operations to storage.
pub fn commit_undo_ops(txn: &mut dyn StorageTxn, mut undo_ops: Vec<ReplicaOp>) -> Result<bool> {
    let mut applied = false;
    let mut local_ops = txn.operations().unwrap();

    // Drop undo_ops iff they're the latest operations.
    // TODO Support concurrent undo by adding the reverse of undo_ops rather than popping from operations.
    let old_len = local_ops.len();
    let undo_len = undo_ops.len();
    let new_len = old_len - undo_len;
    let local_undo_ops = &local_ops[new_len..old_len];
    undo_ops.reverse();
    if local_undo_ops != undo_ops {
        info!("Undo failed: concurrent changes to the database occurred.");
        return Ok(applied);
    }
    undo_ops.reverse();
    local_ops.truncate(new_len);

    for op in undo_ops {
        debug!("Reversing operation {:?}", op);
        let rev_ops = op.reverse_ops();
        for op in rev_ops {
            trace!("Applying reversed operation {:?}", op);
            apply::apply_op(txn, &op)?;
            applied = true;
        }
    }

    if undo_len != 0 {
        txn.set_operations(local_ops)?;
        txn.commit()?;
    }

    Ok(applied)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::server::SyncOp;
    use crate::taskdb::TaskDb;
    use chrono::Utc;
    use pretty_assertions::assert_eq;
    use uuid::Uuid;

    #[test]
    fn test_apply_create() -> Result<()> {
        let mut db = TaskDb::new_inmemory();
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();
        let timestamp = Utc::now();

        // apply a few ops, capture the DB state, make an undo point, and then apply a few more
        // ops.
        db.apply(SyncOp::Create { uuid: uuid1 })?;
        db.apply(SyncOp::Update {
            uuid: uuid1,
            property: "prop".into(),
            value: Some("v1".into()),
            timestamp,
        })?;
        db.apply(SyncOp::Create { uuid: uuid2 })?;
        db.apply(SyncOp::Update {
            uuid: uuid2,
            property: "prop".into(),
            value: Some("v2".into()),
            timestamp,
        })?;
        db.apply(SyncOp::Update {
            uuid: uuid2,
            property: "prop2".into(),
            value: Some("v3".into()),
            timestamp,
        })?;

        let db_state = db.sorted_tasks();

        db.add_undo_point()?;
        db.apply(SyncOp::Delete { uuid: uuid1 })?;
        db.apply(SyncOp::Update {
            uuid: uuid2,
            property: "prop".into(),
            value: None,
            timestamp,
        })?;
        db.apply(SyncOp::Update {
            uuid: uuid2,
            property: "prop2".into(),
            value: Some("new-value".into()),
            timestamp,
        })?;

        assert_eq!(db.operations().len(), 9);

        {
            let mut txn = db.storage.txn()?;
            assert!(undo(txn.as_mut())?);
        }

        // undo took db back to the snapshot
        assert_eq!(db.operations().len(), 5);
        assert_eq!(db.sorted_tasks(), db_state);

        {
            let mut txn = db.storage.txn()?;
            assert!(undo(txn.as_mut())?);
        }

        // empty db
        assert_eq!(db.operations().len(), 0);
        assert_eq!(db.sorted_tasks(), vec![]);

        {
            let mut txn = db.storage.txn()?;
            // nothing left to undo, so undo() returns false
            assert!(!undo(txn.as_mut())?);
        }

        Ok(())
    }
}

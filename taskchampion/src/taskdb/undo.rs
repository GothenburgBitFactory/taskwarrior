use super::apply;
use crate::storage::{ReplicaOp, StorageTxn};
use log::{debug, trace};

/// Undo local operations until an UndoPoint.
pub(super) fn undo(txn: &mut dyn StorageTxn) -> anyhow::Result<bool> {
    let mut applied = false;
    let mut popped = false;
    let mut local_ops = txn.operations()?;

    while let Some(op) = local_ops.pop() {
        popped = true;
        if op == ReplicaOp::UndoPoint {
            break;
        }
        debug!("Reversing operation {:?}", op);
        let rev_ops = op.reverse_ops();
        for op in rev_ops {
            trace!("Applying reversed operation {:?}", op);
            apply::apply_op(txn, &op)?;
            applied = true;
        }
    }

    if popped {
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
    fn test_apply_create() -> anyhow::Result<()> {
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

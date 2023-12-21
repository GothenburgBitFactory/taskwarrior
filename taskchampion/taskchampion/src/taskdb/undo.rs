use super::apply;
use crate::errors::Result;
use crate::server::SyncOp;
use crate::storage::{ReplicaOp, StorageTxn, TaskMap};
use chrono::{DateTime, Utc};
use log::{debug, trace};

/// A representation of the current set of operations which has been split on the UndoPoint, with the newest operations assigned to undo_ops and the remainder of the set assigned to reverted_ops.
/// Undo instances should be short-lived because hold ownership, which is effectively a lock, on the TaskDb. This is a feature though, because if we wrote another change to the database and then applied our reverted_ops, those other changes would be clobbered.
struct Undo<'a> {
    /// The transaction with which to execute the undo.
    txn: &'a mut dyn StorageTxn,

    /// The operations which would be reversed by an undo.
    undo_ops: Vec<ReplicaOp>,

    /// The reverse operations to be applied.
    rev_ops: Vec<SyncOp>,

    /// The set of operations after reversing undo_ops.
    reverted_ops: Vec<ReplicaOp>,
}

/// Undo local operations until the most recent UndoPoint, returning false if there are no
/// local operations to undo.
impl Undo<'_> {
    pub fn new(txn: &mut dyn StorageTxn) -> Undo {
        let mut local_ops = txn.operations().unwrap();
        let mut undo_ops: Vec<ReplicaOp> = Vec::new();
        let mut rev_ops: Vec<SyncOp> = Vec::new();

        while let Some(op) = local_ops.pop() {
            if op == ReplicaOp::UndoPoint {
                break;
            }
            undo_ops.push(op);
        }

        for op in &undo_ops {
            debug!("Reversing operation {:?}", op);
            rev_ops.append(&mut op.clone().reverse_ops());
        }

        Undo {
            txn,
            undo_ops,
            rev_ops,
            reverted_ops: local_ops,
        }
    }

    /// Commit to storage.
    pub fn commit(self) -> Result<bool> {
        let mut applied = false;

        for op in self.rev_ops {
            trace!("Applying reversed operation {:?}", op);
            apply::apply_op(&mut *self.txn, &op)?;
            applied = true;
        }

        if !self.undo_ops.is_empty() {
            self.txn.set_operations(self.reverted_ops)?;
            self.txn.commit()?;
        }

        Ok(applied)
    }
}

#[repr(C)]
pub struct UndoDiff {
    current: TaskMap,
    prior: TaskMap,
    when: DateTime<Utc>,
}

impl UndoDiff {
    fn new(undo: &Undo) -> UndoDiff {
        let mut current = TaskMap::new();
        let mut prior = TaskMap::new();
        let mut when: Option<DateTime<Utc>> = None;

        for op in &undo.undo_ops {
            let sync_op = op.clone().into_sync().unwrap();
            let timestamp = UndoDiff::apply_op(&mut current, &sync_op);
            if when == None {
                when = timestamp;
            }
        }

        for sync_op in &undo.rev_ops {
            UndoDiff::apply_op(&mut prior, sync_op);
        }

        UndoDiff {
            current,
            prior,
            when: when.unwrap(),
        }
    }

    fn apply_op(task: &mut TaskMap, op: &SyncOp) -> Option<DateTime<Utc>> {
        match op {
            SyncOp::Create { uuid } => {
                task.insert("uuid".to_string(), uuid.to_string());
                None
            }
            SyncOp::Delete { .. } => {
                task.clear();
                None
            }
            SyncOp::Update {
                property,
                value: Some(value),
                timestamp,
                ..
            } => {
                task.insert(property.to_string(), value.to_string());
                Some(*timestamp)
            }
            SyncOp::Update {
                property,
                value: None,
                timestamp,
                ..
            } => {
                task.remove(property);
                Some(*timestamp)
            }
        }
    }
}

pub fn undo<F>(txn: &mut dyn StorageTxn, condition: F) -> Result<bool>
where
    F: Fn(UndoDiff) -> bool,
{
    let mut applied = false;
    let proposed_undo = Undo::new(txn);
    let undo_diff = UndoDiff::new(&proposed_undo);

    if condition(undo_diff) {
        applied = proposed_undo.commit()?;
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

use crate::storage::{StorageTxn, TaskMap};
use std::collections::HashSet;

/// Rebuild the working set using a function to identify tasks that should be in the set.  This
/// renumbers the existing working-set tasks to eliminate gaps, and also adds any tasks that
/// are not already in the working set but should be.  The rebuild occurs in a single
/// trasnsaction against the storage backend.
pub fn rebuild<F>(txn: &mut dyn StorageTxn, in_working_set: F, renumber: bool) -> anyhow::Result<()>
where
    F: Fn(&TaskMap) -> bool,
{
    let mut new_ws = vec![None]; // index 0 is always None
    let mut seen = HashSet::new();

    // The goal here is for existing working-set items to be "compressed' down to index 1, so
    // we begin by scanning the current working set and inserting any tasks that should still
    // be in the set into new_ws, implicitly dropping any tasks that are no longer in the
    // working set.
    for elt in txn.get_working_set()?.drain(1..) {
        if let Some(uuid) = elt {
            if let Some(task) = txn.get_task(uuid)? {
                if in_working_set(&task) {
                    new_ws.push(Some(uuid));
                    seen.insert(uuid);
                    continue;
                }
            }
        }

        // if we are not renumbering, then insert a blank working-set entry here
        if !renumber {
            new_ws.push(None);
        }
    }

    // if renumbering, clear the working set and re-add
    if renumber {
        txn.clear_working_set()?;
        for elt in new_ws.drain(1..new_ws.len()).flatten() {
            txn.add_to_working_set(elt)?;
        }
    } else {
        // ..otherwise, just clear the None items determined above from the working set
        for (i, elt) in new_ws.iter().enumerate().skip(1) {
            if elt.is_none() {
                txn.set_working_set_item(i, None)?;
            }
        }
    }

    // Now go hunting for tasks that should be in this list but are not, adding them at the
    // end of the list, whether renumbering or not
    for (uuid, task) in txn.all_tasks()? {
        if !seen.contains(&uuid) && in_working_set(&task) {
            txn.add_to_working_set(uuid)?;
        }
    }

    txn.commit()?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::server::SyncOp;
    use crate::taskdb::TaskDb;
    use chrono::Utc;
    use uuid::Uuid;

    #[test]
    fn rebuild_working_set_renumber() -> anyhow::Result<()> {
        rebuild_working_set(true)
    }

    #[test]
    fn rebuild_working_set_no_renumber() -> anyhow::Result<()> {
        rebuild_working_set(false)
    }

    fn rebuild_working_set(renumber: bool) -> anyhow::Result<()> {
        let mut db = TaskDb::new_inmemory();
        let mut uuids = vec![];
        uuids.push(Uuid::new_v4());
        println!("uuids[0]: {:?} - pending, not in working set", uuids[0]);
        uuids.push(Uuid::new_v4());
        println!("uuids[1]: {:?} - pending, in working set", uuids[1]);
        uuids.push(Uuid::new_v4());
        println!("uuids[2]: {:?} - not pending, not in working set", uuids[2]);
        uuids.push(Uuid::new_v4());
        println!("uuids[3]: {:?} - not pending, in working set", uuids[3]);
        uuids.push(Uuid::new_v4());
        println!("uuids[4]: {:?} - pending, in working set", uuids[4]);

        // add everything to the TaskDb
        for uuid in &uuids {
            db.apply(SyncOp::Create { uuid: *uuid })?;
        }
        for i in &[0usize, 1, 4] {
            db.apply(SyncOp::Update {
                uuid: uuids[*i].clone(),
                property: String::from("status"),
                value: Some("pending".into()),
                timestamp: Utc::now(),
            })?;
        }

        // set the existing working_set as we want it
        {
            let mut txn = db.storage.txn()?;
            txn.clear_working_set()?;

            for i in &[1usize, 3, 4] {
                txn.add_to_working_set(uuids[*i])?;
            }

            txn.commit()?;
        }

        assert_eq!(
            db.working_set()?,
            vec![
                None,
                Some(uuids[1].clone()),
                Some(uuids[3].clone()),
                Some(uuids[4].clone())
            ]
        );

        rebuild(
            db.storage.txn()?.as_mut(),
            |t| {
                if let Some(status) = t.get("status") {
                    status == "pending"
                } else {
                    false
                }
            },
            renumber,
        )?;

        let exp = if renumber {
            // uuids[1] and uuids[4] are already in the working set, so are compressed
            // to the top, and then uuids[0] is added.
            vec![
                None,
                Some(uuids[1].clone()),
                Some(uuids[4].clone()),
                Some(uuids[0].clone()),
            ]
        } else {
            // uuids[1] and uuids[4] are already in the working set, at indexes 1 and 3,
            // and then uuids[0] is added.
            vec![
                None,
                Some(uuids[1].clone()),
                None,
                Some(uuids[4].clone()),
                Some(uuids[0].clone()),
            ]
        };

        assert_eq!(db.working_set()?, exp);

        Ok(())
    }
}

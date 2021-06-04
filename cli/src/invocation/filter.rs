use crate::argparse::{Condition, Filter, TaskId};
use std::collections::HashSet;
use taskchampion::{Replica, Status, Task, Uuid, WorkingSet};

fn match_task(filter: &Filter, task: &Task, uuid: Uuid, working_set: &WorkingSet) -> bool {
    for cond in &filter.conditions {
        match cond {
            Condition::HasTag(ref tag) => {
                if !task.has_tag(tag) {
                    return false;
                }
            }
            Condition::NoTag(ref tag) => {
                if task.has_tag(tag) {
                    return false;
                }
            }
            Condition::Status(status) => {
                if task.get_status() != *status {
                    return false;
                }
            }
            Condition::IdList(ids) => {
                let uuid_str = uuid.to_string();
                let mut found = false;
                let working_set_id = working_set.by_uuid(uuid);

                for id in ids {
                    if match id {
                        TaskId::WorkingSetId(i) => Some(*i) == working_set_id,
                        TaskId::PartialUuid(partial) => uuid_str.starts_with(partial),
                        TaskId::Uuid(i) => *i == uuid,
                    } {
                        found = true;
                        break;
                    }
                }
                if !found {
                    return false;
                }
            }
        }
    }
    true
}

// the universe of tasks we must consider
enum Universe {
    /// Scan all the tasks
    AllTasks,
    /// Scan the working set (for pending tasks)
    WorkingSet,
    /// Scan an explicit set of tasks, "Absolute" meaning either full UUID or a working set
    /// index
    AbsoluteIdList(Vec<TaskId>),
}

/// Determine the universe for the given filter; avoiding the need to scan all tasks in most cases.
fn universe_for_filter(filter: &Filter) -> Universe {
    /// If there is a condition with Status::Pending, return true
    fn has_pending_condition(filter: &Filter) -> bool {
        filter
            .conditions
            .iter()
            .any(|cond| matches!(cond, Condition::Status(Status::Pending)))
    }

    /// If there is a condition with an IdList containing no partial UUIDs,
    /// return that.
    fn absolute_id_list_condition(filter: &Filter) -> Option<Vec<TaskId>> {
        filter
            .conditions
            .iter()
            .find(|cond| {
                if let Condition::IdList(ids) = cond {
                    !ids.iter().any(|id| matches!(id, TaskId::PartialUuid(_)))
                } else {
                    false
                }
            })
            .map(|cond| {
                if let Condition::IdList(ids) = cond {
                    ids.to_vec()
                } else {
                    unreachable!() // any condition found above must be an IdList(_)
                }
            })
    }

    if let Some(ids) = absolute_id_list_condition(filter) {
        Universe::AbsoluteIdList(ids)
    } else if has_pending_condition(filter) {
        Universe::WorkingSet
    } else {
        Universe::AllTasks
    }
}

/// Return the tasks matching the given filter.  This will return each matching
/// task once, even if the user specified the same task multiple times on the
/// command line.
pub(super) fn filtered_tasks(
    replica: &mut Replica,
    filter: &Filter,
) -> anyhow::Result<impl Iterator<Item = Task>> {
    let mut res = vec![];

    log::debug!("Applying filter {:?}", filter);

    let working_set = replica.working_set()?;

    // We will enumerate the universe of tasks for this filter, checking
    // each resulting task with match_task
    match universe_for_filter(filter) {
        // A list of IDs, but some are partial so we need to iterate over
        // all tasks and pattern-match their Uuids
        Universe::AbsoluteIdList(ref ids) => {
            log::debug!("Scanning only the tasks specified in the filter");
            // this is the only case where we might accidentally return the same task
            // several times, so we must track the seen tasks.
            let mut seen = HashSet::new();
            for id in ids {
                let task = match id {
                    TaskId::WorkingSetId(id) => working_set
                        .by_index(*id)
                        .map(|uuid| replica.get_task(uuid))
                        .transpose()?
                        .flatten(),
                    TaskId::PartialUuid(_) => unreachable!(), // not present in absolute id list
                    TaskId::Uuid(id) => replica.get_task(*id)?,
                };

                if let Some(task) = task {
                    // if we have already seen this task, skip ahead..
                    let uuid = task.get_uuid();
                    if seen.contains(&uuid) {
                        continue;
                    }
                    seen.insert(uuid);

                    if match_task(filter, &task, uuid, &working_set) {
                        res.push(task);
                    }
                }
            }
        }

        // All tasks -- iterate over the full set
        Universe::AllTasks => {
            log::debug!("Scanning all tasks in the task database");
            for (uuid, task) in replica.all_tasks()?.drain() {
                if match_task(filter, &task, uuid, &working_set) {
                    res.push(task);
                }
            }
        }
        Universe::WorkingSet => {
            log::debug!("Scanning only the working set (pending tasks)");
            for (_, uuid) in working_set.iter() {
                if let Some(task) = replica.get_task(uuid)? {
                    if match_task(filter, &task, uuid, &working_set) {
                        res.push(task);
                    }
                }
            }
        }
    }
    Ok(res.into_iter())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use taskchampion::Status;

    #[test]
    fn exact_ids() {
        let mut replica = test_replica();

        let t1 = replica.new_task(Status::Pending, s!("A")).unwrap();
        let t2 = replica.new_task(Status::Completed, s!("B")).unwrap();
        let _t = replica.new_task(Status::Pending, s!("C")).unwrap();
        replica.rebuild_working_set(true).unwrap();

        let t1uuid = t1.get_uuid();

        let filter = Filter {
            conditions: vec![Condition::IdList(vec![
                TaskId::Uuid(t1uuid),        // A
                TaskId::WorkingSetId(1),     // A (again, dups filtered)
                TaskId::Uuid(t2.get_uuid()), // B
            ])],
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A"), s!("B")], filtered);
    }

    #[test]
    fn partial_ids() {
        let mut replica = test_replica();

        let t1 = replica.new_task(Status::Pending, s!("A")).unwrap();
        let t2 = replica.new_task(Status::Completed, s!("B")).unwrap();
        let _t = replica.new_task(Status::Pending, s!("C")).unwrap();
        replica.rebuild_working_set(true).unwrap();

        let t1uuid = t1.get_uuid();
        let t2uuid = t2.get_uuid().to_string();
        let t2partial = t2uuid[..13].to_owned();

        let filter = Filter {
            conditions: vec![Condition::IdList(vec![
                TaskId::Uuid(t1uuid),           // A
                TaskId::WorkingSetId(1),        // A (again, dups filtered)
                TaskId::PartialUuid(t2partial), // B
            ])],
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A"), s!("B")], filtered);
    }

    #[test]
    fn all_tasks() {
        let mut replica = test_replica();

        replica.new_task(Status::Pending, s!("A")).unwrap();
        replica.new_task(Status::Completed, s!("B")).unwrap();
        replica.new_task(Status::Deleted, s!("C")).unwrap();
        replica.rebuild_working_set(true).unwrap();

        let filter = Filter { conditions: vec![] };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A"), s!("B"), s!("C")], filtered);
    }

    #[test]
    fn tag_filtering() -> anyhow::Result<()> {
        let mut replica = test_replica();
        let yes = tag!("yes");
        let no = tag!("no");

        let mut t1 = replica
            .new_task(Status::Pending, s!("A"))?
            .into_mut(&mut replica);
        t1.add_tag(&yes)?;
        let mut t2 = replica
            .new_task(Status::Pending, s!("B"))?
            .into_mut(&mut replica);
        t2.add_tag(&yes)?;
        t2.add_tag(&no)?;
        let mut t3 = replica
            .new_task(Status::Pending, s!("C"))?
            .into_mut(&mut replica);
        t3.add_tag(&no)?;
        let _t4 = replica.new_task(Status::Pending, s!("D"))?;

        // look for just "yes" (A and B)
        let filter = Filter {
            conditions: vec![Condition::HasTag(tag!("yes"))],
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)?
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A"), s!("B")], filtered);

        // look for tags without "no" (A, D)
        let filter = Filter {
            conditions: vec![Condition::NoTag(tag!("no"))],
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)?
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A"), s!("D")], filtered);

        // look for tags with "yes" and "no" (B)
        let filter = Filter {
            conditions: vec![
                Condition::HasTag(tag!("yes")),
                Condition::HasTag(tag!("no")),
            ],
        };
        let filtered: Vec<_> = filtered_tasks(&mut replica, &filter)?
            .map(|t| t.get_description().to_owned())
            .collect();
        assert_eq!(vec![s!("B")], filtered);

        Ok(())
    }

    #[test]
    fn pending_tasks() {
        let mut replica = test_replica();

        replica.new_task(Status::Pending, s!("A")).unwrap();
        replica.new_task(Status::Completed, s!("B")).unwrap();
        replica.new_task(Status::Deleted, s!("C")).unwrap();
        replica.rebuild_working_set(true).unwrap();

        let filter = Filter {
            conditions: vec![Condition::Status(Status::Pending)],
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec![s!("A")], filtered);
    }
}

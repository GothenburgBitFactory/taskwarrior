use crate::argparse::{Filter, TaskId, Universe};
use failure::Fallible;
use std::collections::HashSet;
use taskchampion::{Replica, Task};

fn match_task(_filter: &Filter, _task: &Task) -> bool {
    // TODO: at the moment, only filtering by Universe is supported
    true
}

/// Return the tasks matching the given filter.  This will return each matching
/// task once, even if the user specified the same task multiple times on the
/// command line.
pub(super) fn filtered_tasks(
    replica: &mut Replica,
    filter: &Filter,
) -> Fallible<impl Iterator<Item = Task>> {
    let mut res = vec![];

    fn is_partial_uuid(taskid: &TaskId) -> bool {
        match taskid {
            TaskId::PartialUuid(_) => true,
            _ => false,
        }
    }

    // We will enumerate the universe of tasks for this filter, checking
    // each resulting task with match_task
    match filter.universe {
        // A list of IDs, but some are partial so we need to iterate over
        // all tasks and pattern-match their Uuids
        Universe::IdList(ref ids) if ids.iter().any(is_partial_uuid) => {
            'task: for (uuid, task) in replica.all_tasks()?.drain() {
                for id in ids {
                    if match id {
                        TaskId::WorkingSetId(id) => {
                            // NOTE: (#108) this results in many reads of the working set; it
                            // may be better to cache this information here or in the Replica.
                            replica.get_working_set_index(&uuid)? == Some(*id)
                        }
                        TaskId::PartialUuid(prefix) => uuid.to_string().starts_with(prefix),
                        TaskId::Uuid(id) => id == &uuid,
                    } {
                        if match_task(filter, &task) {
                            res.push(task);
                            continue 'task;
                        }
                    }
                }
            }
        }

        // A list of full IDs, which we can fetch directly
        Universe::IdList(ref ids) => {
            // this is the only case where we might accidentally return the same task
            // several times, so we must track the seen tasks.
            let mut seen = HashSet::new();
            for id in ids {
                let task = match id {
                    TaskId::WorkingSetId(id) => replica.get_working_set_task(*id)?,
                    TaskId::PartialUuid(_) => unreachable!(), // handled above
                    TaskId::Uuid(id) => replica.get_task(id)?,
                };

                if let Some(task) = task {
                    // if we have already seen this task, skip ahead..
                    let uuid = *task.get_uuid();
                    if seen.contains(&uuid) {
                        continue;
                    }
                    seen.insert(uuid);

                    if match_task(filter, &task) {
                        res.push(task);
                    }
                }
            }
        }

        // All tasks -- iterate over the full set
        Universe::AllTasks => {
            for (_, task) in replica.all_tasks()?.drain() {
                if match_task(filter, &task) {
                    res.push(task);
                }
            }
        }

        // Pending tasks -- just scan the working set
        Universe::PendingTasks => {
            for task in replica.working_set()?.drain(..) {
                if let Some(task) = task {
                    if match_task(filter, &task) {
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

        let t1 = replica.new_task(Status::Pending, "A".to_owned()).unwrap();
        let t2 = replica.new_task(Status::Completed, "B".to_owned()).unwrap();
        let _t = replica.new_task(Status::Pending, "C".to_owned()).unwrap();
        replica.gc().unwrap();

        let t1uuid = *t1.get_uuid();

        let filter = Filter {
            universe: Universe::IdList(vec![
                TaskId::Uuid(t1uuid),         // A
                TaskId::WorkingSetId(1),      // A (again, dups filtered)
                TaskId::Uuid(*t2.get_uuid()), // B
            ]),
            ..Default::default()
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec!["A".to_owned(), "B".to_owned()], filtered);
    }

    #[test]
    fn partial_ids() {
        let mut replica = test_replica();

        let t1 = replica.new_task(Status::Pending, "A".to_owned()).unwrap();
        let t2 = replica.new_task(Status::Completed, "B".to_owned()).unwrap();
        let _t = replica.new_task(Status::Pending, "C".to_owned()).unwrap();
        replica.gc().unwrap();

        let t1uuid = *t1.get_uuid();
        let t2uuid = t2.get_uuid().to_string();
        let t2partial = t2uuid[..13].to_owned();

        let filter = Filter {
            universe: Universe::IdList(vec![
                TaskId::Uuid(t1uuid),           // A
                TaskId::WorkingSetId(1),        // A (again, dups filtered)
                TaskId::PartialUuid(t2partial), // B
            ]),
            ..Default::default()
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec!["A".to_owned(), "B".to_owned()], filtered);
    }

    #[test]
    fn all_tasks() {
        let mut replica = test_replica();

        replica.new_task(Status::Pending, "A".to_owned()).unwrap();
        replica.new_task(Status::Completed, "B".to_owned()).unwrap();
        replica.new_task(Status::Deleted, "C".to_owned()).unwrap();
        replica.gc().unwrap();

        let filter = Filter {
            universe: Universe::AllTasks,
            ..Default::default()
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(
            vec!["A".to_owned(), "B".to_owned(), "C".to_owned()],
            filtered
        );
    }

    #[test]
    fn pending_tasks() {
        let mut replica = test_replica();

        replica.new_task(Status::Pending, "A".to_owned()).unwrap();
        replica.new_task(Status::Completed, "B".to_owned()).unwrap();
        replica.new_task(Status::Deleted, "C".to_owned()).unwrap();
        replica.gc().unwrap();

        let filter = Filter {
            universe: Universe::PendingTasks,
            ..Default::default()
        };
        let mut filtered: Vec<_> = filtered_tasks(&mut replica, &filter)
            .unwrap()
            .map(|t| t.get_description().to_owned())
            .collect();
        filtered.sort();
        assert_eq!(vec!["A".to_owned()], filtered);
    }
}

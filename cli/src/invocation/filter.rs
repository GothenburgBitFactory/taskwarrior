use crate::argparse::Filter;
use failure::Fallible;
use taskchampion::{Replica, Task};

/// Return the tasks matching the given filter.
pub(super) fn filtered_tasks(
    replica: &mut Replica,
    filter: &Filter,
) -> Fallible<impl Iterator<Item = Task>> {
    // For the moment, this gets the entire set of tasks and then iterates
    // over the result.  A few optimizations are possible:
    //
    //  - id_list could be better parsed (id, uuid-fragment, uuid) in argparse
    //  - depending on the nature of the filter, we could just scan the working set
    //  - we could produce the tasks on-demand (but at the cost of holding a ref
    //    to the replica, preventing modifying tasks..)
    let mut res = vec![];
    'task: for (uuid, task) in replica.all_tasks()?.drain() {
        if let Some(ref ids) = filter.id_list {
            for id in ids {
                if let Ok(index) = id.parse::<usize>() {
                    if replica.get_working_set_index(&uuid)? == Some(index) {
                        res.push(task);
                        continue 'task;
                    }
                } else if uuid.to_string().starts_with(id) {
                    res.push(task);
                    continue 'task;
                }
            }
        } else {
            // default to returning all tasks
            res.push(task);
            continue 'task;
        }
    }
    Ok(res.into_iter())
}

use crate::argparse::Filter;
use crate::invocation::filtered_tasks;
use crate::table;
use prettytable::{cell, row, Table};
use taskchampion::{Replica, Status};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    filter: Filter,
    debug: bool,
) -> Result<(), crate::Error> {
    let working_set = replica.working_set()?;

    for task in filtered_tasks(replica, &filter)? {
        let uuid = task.get_uuid();

        let mut t = Table::new();
        t.set_format(table::format());
        if debug {
            t.set_titles(row![b->"key", b->"value"]);
            for (k, v) in task.get_taskmap().iter() {
                t.add_row(row![k, v]);
            }
        } else {
            t.add_row(row![b->"Uuid", uuid]);
            if let Some(i) = working_set.by_uuid(uuid) {
                t.add_row(row![b->"Id", i]);
            }
            t.add_row(row![b->"Description", task.get_description()]);
            t.add_row(row![b->"Status", task.get_status()]);
            t.add_row(row![b->"Active", task.is_active()]);
            let mut tags: Vec<_> = task.get_tags().map(|t| format!("+{}", t)).collect();
            if !tags.is_empty() {
                tags.sort();
                t.add_row(row![b->"Tags", tags.join(" ")]);
            }
            if let Some(wait) = task.get_wait() {
                t.add_row(row![b->"Wait", wait]);
            }
            let mut annotations: Vec<_> = task.get_annotations().collect();
            annotations.sort();
            for ann in annotations {
                t.add_row(row![b->"Annotation", format!("{}: {}", ann.entry, ann.description)]);
            }

            let mut deps: Vec<_> = task.get_dependencies().collect();
            deps.sort();
            for dep in deps {
                let mut descr = None;
                if let Some(task) = replica.get_task(dep)? {
                    if task.get_status() == Status::Pending {
                        if let Some(i) = working_set.by_uuid(dep) {
                            descr = Some(format!("{} - {}", i, task.get_description()))
                        } else {
                            descr = Some(format!("{} - {}", dep, task.get_description()))
                        }
                    }
                }

                if let Some(descr) = descr {
                    t.add_row(row![b->"Depends On", descr]);
                }
            }
        }
        t.print(w)?;
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::{Condition, TaskId};
    use crate::invocation::test::*;

    use taskchampion::Status;

    #[test]
    fn test_info() {
        let mut w = test_writer();
        let mut replica = test_replica();
        replica.new_task(Status::Pending, s!("my task")).unwrap();

        let filter = Filter {
            ..Default::default()
        };
        let debug = false;
        execute(&mut w, &mut replica, filter, debug).unwrap();
        assert!(w.into_string().contains("my task"));
    }

    #[test]
    fn test_deps() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let t1 = replica.new_task(Status::Pending, s!("my task")).unwrap();
        let t2 = replica
            .new_task(Status::Pending, s!("dunno, depends"))
            .unwrap();
        let mut t2 = t2.into_mut(&mut replica);
        t2.add_dependency(t1.get_uuid()).unwrap();
        let t2 = t2.into_immut();

        let filter = Filter {
            conditions: vec![Condition::IdList(vec![TaskId::Uuid(t2.get_uuid())])],
        };
        let debug = false;
        execute(&mut w, &mut replica, filter, debug).unwrap();
        let s = w.into_string();
        // length of whitespace between these two strings is not important
        assert!(s.contains("Depends On"));
        assert!(s.contains("1 - my task"));
    }
}

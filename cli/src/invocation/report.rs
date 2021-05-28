use crate::argparse::Filter;
use crate::invocation::filtered_tasks;
use crate::settings::{Column, Property, Report, Settings, SortBy};
use crate::table;
use anyhow::anyhow;
use prettytable::{Row, Table};
use std::cmp::Ordering;
use taskchampion::{Replica, Task, WorkingSet};
use termcolor::WriteColor;

/// Sort tasks for the given report.
fn sort_tasks(tasks: &mut Vec<Task>, report: &Report, working_set: &WorkingSet) {
    tasks.sort_by(|a, b| {
        for s in &report.sort {
            let ord = match s.sort_by {
                SortBy::Id => {
                    let a_uuid = a.get_uuid();
                    let b_uuid = b.get_uuid();
                    let a_id = working_set.by_uuid(a_uuid);
                    let b_id = working_set.by_uuid(b_uuid);
                    match (a_id, b_id) {
                        (Some(a_id), Some(b_id)) => a_id.cmp(&b_id),
                        (Some(_), None) => Ordering::Less,
                        (None, Some(_)) => Ordering::Greater,
                        (None, None) => a_uuid.cmp(&b_uuid),
                    }
                }
                SortBy::Uuid => a.get_uuid().cmp(&b.get_uuid()),
                SortBy::Description => a.get_description().cmp(b.get_description()),
            };
            // If this sort property is equal, go on to the next..
            if ord == Ordering::Equal {
                continue;
            }
            // Reverse order if not ascending
            if s.ascending {
                return ord;
            } else {
                return ord.reverse();
            }
        }
        Ordering::Equal
    });
}

/// Generate the string representation for the given task and column.
fn task_column(task: &Task, column: &Column, working_set: &WorkingSet) -> String {
    match column.property {
        Property::Id => {
            let uuid = task.get_uuid();
            let mut id = uuid.to_string();
            if let Some(i) = working_set.by_uuid(uuid) {
                id = i.to_string();
            }
            id
        }
        Property::Uuid => {
            let uuid = task.get_uuid();
            uuid.to_string()
        }
        Property::Active => match task.is_active() {
            true => "*".to_owned(),
            false => "".to_owned(),
        },
        Property::Description => task.get_description().to_owned(),
        Property::Tags => {
            let mut tags = task
                .get_tags()
                .map(|t| format!("+{}", t))
                .collect::<Vec<_>>();
            tags.sort();
            tags.join(" ")
        }
    }
}

pub(super) fn display_report<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    settings: &Settings,
    report_name: String,
    filter: Filter,
) -> anyhow::Result<()> {
    let mut t = Table::new();
    let working_set = replica.working_set()?;

    // Get the report from settings
    let mut report = settings
        .reports
        .get(&report_name)
        .ok_or_else(|| anyhow!("report `{}` not defined", report_name))?
        .clone();

    // include any user-supplied filter conditions
    report.filter = report.filter.intersect(filter);

    // Get the tasks from the filter
    let mut tasks: Vec<_> = filtered_tasks(replica, &report.filter)?.collect();

    // ..sort them as desired
    sort_tasks(&mut tasks, &report, &working_set);

    // ..set up the column titles
    t.set_format(table::format());
    t.set_titles(report.columns.iter().map(|col| col.label.clone()).into());

    // ..insert the data
    for task in &tasks {
        let row: Row = report
            .columns
            .iter()
            .map(|col| task_column(task, col, &working_set))
            .collect::<Row>();
        t.add_row(row);
    }

    // ..and display it
    t.print(w)?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use crate::settings::Sort;
    use std::convert::TryInto;
    use taskchampion::{Status, Uuid};

    fn create_tasks(replica: &mut Replica) -> [Uuid; 3] {
        let t1 = replica.new_task(Status::Pending, s!("A")).unwrap();
        let t2 = replica.new_task(Status::Pending, s!("B")).unwrap();
        let t3 = replica.new_task(Status::Pending, s!("C")).unwrap();

        // t2 is comleted and not in the working set
        let mut t2 = t2.into_mut(replica);
        t2.set_status(Status::Completed).unwrap();
        let t2 = t2.into_immut();

        replica.rebuild_working_set(true).unwrap();

        [t1.get_uuid(), t2.get_uuid(), t3.get_uuid()]
    }

    #[test]
    fn sorting_by_descr() {
        let mut replica = test_replica();
        create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();
        let mut report = Report {
            sort: vec![Sort {
                ascending: true,
                sort_by: SortBy::Description,
            }],
            ..Default::default()
        };

        // ascending
        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let descriptions: Vec<_> = tasks.iter().map(|t| t.get_description()).collect();
        assert_eq!(descriptions, vec!["A", "B", "C"]);

        // ascending
        report.sort[0].ascending = false;
        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let descriptions: Vec<_> = tasks.iter().map(|t| t.get_description()).collect();
        assert_eq!(descriptions, vec!["C", "B", "A"]);
    }

    #[test]
    fn sorting_by_id() {
        let mut replica = test_replica();
        create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();
        let mut report = Report {
            sort: vec![Sort {
                ascending: true,
                sort_by: SortBy::Id,
            }],
            ..Default::default()
        };

        // ascending
        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let descriptions: Vec<_> = tasks.iter().map(|t| t.get_description()).collect();
        assert_eq!(descriptions, vec!["A", "C", "B"]);

        // ascending
        report.sort[0].ascending = false;
        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let descriptions: Vec<_> = tasks.iter().map(|t| t.get_description()).collect();
        assert_eq!(descriptions, vec!["B", "C", "A"]);
    }

    #[test]
    fn sorting_by_uuid() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();
        let report = Report {
            sort: vec![Sort {
                ascending: true,
                sort_by: SortBy::Uuid,
            }],
            ..Default::default()
        };

        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let got_uuids: Vec<_> = tasks.iter().map(|t| t.get_uuid()).collect();
        let mut exp_uuids = uuids.to_vec();
        exp_uuids.sort();
        assert_eq!(got_uuids, exp_uuids);
    }

    #[test]
    fn sorting_by_multiple() {
        let mut replica = test_replica();
        create_tasks(&mut replica);

        // make a second task named A with a larger ID than the first
        let t = replica.new_task(Status::Pending, s!("A")).unwrap();
        t.into_mut(&mut replica)
            .add_tag(&("second".try_into().unwrap()))
            .unwrap();

        let working_set = replica.working_set().unwrap();
        let report = Report {
            sort: vec![
                Sort {
                    ascending: false,
                    sort_by: SortBy::Description,
                },
                Sort {
                    ascending: true,
                    sort_by: SortBy::Id,
                },
            ],
            ..Default::default()
        };

        let mut tasks: Vec<_> = replica.all_tasks().unwrap().values().cloned().collect();
        sort_tasks(&mut tasks, &report, &working_set);
        let descriptions: Vec<_> = tasks.iter().map(|t| t.get_description()).collect();
        assert_eq!(descriptions, vec!["C", "B", "A", "A"]);
        assert!(tasks[3].has_tag(&("second".try_into().unwrap())));
    }

    #[test]
    fn task_column_id() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();

        let task = replica.get_task(uuids[0]).unwrap().unwrap();
        let column = Column {
            label: s!(""),
            property: Property::Id,
        };
        assert_eq!(task_column(&task, &column, &working_set), s!("1"));

        // get the task that's not in the working set, which should show
        // a uuid for its id column
        let task = replica.get_task(uuids[1]).unwrap().unwrap();
        assert_eq!(
            task_column(&task, &column, &working_set),
            uuids[1].to_string()
        );
    }

    #[test]
    fn task_column_uuid() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();

        let task = replica.get_task(uuids[0]).unwrap().unwrap();
        let column = Column {
            label: s!(""),
            property: Property::Uuid,
        };
        assert_eq!(
            task_column(&task, &column, &working_set),
            task.get_uuid().to_string()
        );
    }

    #[test]
    fn task_column_active() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();

        // make task A active
        replica
            .get_task(uuids[0])
            .unwrap()
            .unwrap()
            .into_mut(&mut replica)
            .start()
            .unwrap();

        let column = Column {
            label: s!(""),
            property: Property::Active,
        };

        let task = replica.get_task(uuids[0]).unwrap().unwrap();
        assert_eq!(task_column(&task, &column, &working_set), s!("*"));
        let task = replica.get_task(uuids[2]).unwrap().unwrap();
        assert_eq!(task_column(&task, &column, &working_set), s!(""));
    }

    #[test]
    fn task_column_description() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();

        let task = replica.get_task(uuids[2]).unwrap().unwrap();
        let column = Column {
            label: s!(""),
            property: Property::Description,
        };
        assert_eq!(task_column(&task, &column, &working_set), s!("C"));
    }

    #[test]
    fn task_column_tags() {
        let mut replica = test_replica();
        let uuids = create_tasks(&mut replica);
        let working_set = replica.working_set().unwrap();

        // add some tags to task A
        let mut t1 = replica
            .get_task(uuids[0])
            .unwrap()
            .unwrap()
            .into_mut(&mut replica);
        t1.add_tag(&("foo".try_into().unwrap())).unwrap();
        t1.add_tag(&("bar".try_into().unwrap())).unwrap();

        let column = Column {
            label: s!(""),
            property: Property::Tags,
        };

        let task = replica.get_task(uuids[0]).unwrap().unwrap();
        assert_eq!(task_column(&task, &column, &working_set), s!("+bar +foo"));
        let task = replica.get_task(uuids[2]).unwrap().unwrap();
        assert_eq!(task_column(&task, &column, &working_set), s!(""));
    }
}

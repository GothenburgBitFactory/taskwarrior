use crate::argparse::Report;
use crate::invocation::filtered_tasks;
use crate::table;
use failure::Fallible;
use prettytable::{cell, row, Table};
use taskchampion::Replica;

pub(crate) fn execute(replica: &mut Replica, report: Report) -> Fallible<()> {
    let mut t = Table::new();
    t.set_format(table::format());
    t.set_titles(row![b->"id", b->"act", b->"description"]);
    for task in filtered_tasks(replica, &report.filter)? {
        let uuid = task.get_uuid();
        let mut id = uuid.to_string();
        if let Some(i) = replica.get_working_set_index(&uuid)? {
            id = i.to_string();
        }
        let active = match task.is_active() {
            true => "*",
            false => "",
        };
        t.add_row(row![id, active, task.get_description()]);
    }
    t.printstd();
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::Filter;
    use crate::invocation::cmd::test::test_replica;

    #[test]
    fn test_list() {
        let mut replica = test_replica();
        let report = Report {
            filter: Filter {
                ..Default::default()
            },
        };
        execute(&mut replica, report).unwrap();
        // output is to stdout, so this is as much as we can check
    }
}

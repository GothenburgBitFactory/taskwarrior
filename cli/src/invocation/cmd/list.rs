use crate::argparse::Report;
use crate::invocation::filtered_tasks;
use crate::table;
use failure::Fallible;
use prettytable::{cell, row, Table};
use taskchampion::Replica;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    report: Report,
) -> Fallible<()> {
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
    t.print(w)?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::Filter;
    use crate::invocation::test::*;
    use taskchampion::Status;

    #[test]
    fn test_list() {
        let mut w = test_writer();
        let mut replica = test_replica();
        replica.new_task(Status::Pending, s!("my task")).unwrap();

        let report = Report {
            filter: Filter {
                ..Default::default()
            },
        };
        execute(&mut w, &mut replica, report).unwrap();
        assert!(w.into_string().contains("my task"));
    }
}

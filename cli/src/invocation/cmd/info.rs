use crate::argparse::Filter;
use crate::invocation::filtered_tasks;
use crate::table;
use prettytable::{cell, row, Table};
use taskchampion::Replica;
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
        }
        t.print(w)?;
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
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
}

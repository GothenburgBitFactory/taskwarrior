use crate::argparse::Filter;
use crate::invocation::filtered_tasks;
use crate::table;
use failure::Fallible;
use prettytable::{cell, row, Table};
use taskchampion::Replica;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    filter: Filter,
    debug: bool,
) -> Fallible<()> {
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
            if let Some(i) = replica.get_working_set_index(uuid)? {
                t.add_row(row![b->"Id", i]);
            }
            t.add_row(row![b->"Description", task.get_description()]);
            t.add_row(row![b->"Status", task.get_status()]);
            t.add_row(row![b->"Active", task.is_active()]);
        }
        t.print(w)?;
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::cmd::test::*;
    use taskchampion::Status;

    #[test]
    fn test_info() {
        let mut w = test_writer();
        let mut replica = test_replica();
        replica
            .new_task(Status::Pending, "my task".to_owned())
            .unwrap();

        let filter = Filter {
            ..Default::default()
        };
        let debug = false;
        execute(&mut w, &mut replica, filter, debug).unwrap();
        assert!(w.into_string().contains("my task"));
    }
}

use crate::argparse::Filter;
use crate::invocation::filtered_tasks;
use crate::table;
use failure::Fallible;
use prettytable::{cell, row, Table};
use taskchampion::Replica;

pub(crate) fn execute(replica: &mut Replica, filter: Filter, debug: bool) -> Fallible<()> {
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
        t.printstd();
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::cmd::test::test_replica;

    #[test]
    fn test_info() {
        let mut replica = test_replica();
        let filter = Filter {
            ..Default::default()
        };
        let debug = false;
        execute(&mut replica, filter, debug).unwrap();
        // output is to stdout, so this is as much as we can check
    }
}

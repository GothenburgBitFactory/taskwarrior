use crate::argparse::Report;
use crate::invocation::display_report;
use failure::Fallible;
use taskchampion::Replica;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    report: Report,
) -> Fallible<()> {
    display_report(w, replica, &report)
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::{Column, Filter, Property};
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
            columns: vec![Column {
                label: "Description".to_owned(),
                property: Property::Description,
            }],
            ..Default::default()
        };
        execute(&mut w, &mut replica, report).unwrap();
        assert!(w.into_string().contains("my task"));
    }
}

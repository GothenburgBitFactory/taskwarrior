use crate::argparse::Filter;
use crate::invocation::display_report;
use crate::settings::Settings;
use taskchampion::Replica;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    settings: &Settings,
    report_name: String,
    filter: Filter,
) -> Result<(), crate::Error> {
    display_report(w, replica, settings, report_name, filter)
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::Filter;
    use crate::invocation::test::*;
    use taskchampion::Status;

    #[test]
    fn test_report() {
        let mut w = test_writer();
        let mut replica = test_replica();
        replica.new_task(Status::Pending, s!("my task")).unwrap();

        // The function being tested is only one line long, so this is sort of an integration test
        // for display_report.

        let settings = Default::default();
        let report_name = "next".to_owned();
        let filter = Filter {
            ..Default::default()
        };

        execute(&mut w, &mut replica, &settings, report_name, filter).unwrap();
        assert!(w.into_string().contains("my task"));
    }
}

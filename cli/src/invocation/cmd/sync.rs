use crate::settings::Settings;
use taskchampion::{server::Server, Replica};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    settings: &Settings,
    server: &mut Box<dyn Server>,
) -> Result<(), crate::Error> {
    replica.sync(server, settings.avoid_snapshots)?;
    writeln!(w, "sync complete.")?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;
    use tempfile::TempDir;

    #[test]
    fn test_add() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let server_dir = TempDir::new().unwrap();
        let mut server = test_server(&server_dir);
        let settings = Settings::default();

        // Note that the details of the actual sync are tested thoroughly in the taskchampion crate
        execute(&mut w, &mut replica, &settings, &mut server).unwrap();
        assert_eq!(&w.into_string(), "sync complete.\n")
    }
}

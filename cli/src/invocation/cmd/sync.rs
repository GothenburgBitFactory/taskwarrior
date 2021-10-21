use crate::settings::Settings;
use taskchampion::{server::Server, Error as TCError, Replica};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    settings: &Settings,
    server: &mut Box<dyn Server>,
) -> Result<(), crate::Error> {
    match replica.sync(server, settings.avoid_snapshots) {
        Ok(()) => {
            writeln!(w, "sync complete.")?;
            Ok(())
        }
        Err(e) => match e.downcast() {
            Ok(TCError::OutOfSync) => {
                writeln!(w, "This replica cannot be synchronized with the server.")?;
                writeln!(
                    w,
                    "It may be too old, or some other failure may have occurred."
                )?;
                writeln!(
                    w,
                    "To start fresh, remove the local task database and run `ta sync` again."
                )?;
                writeln!(
                    w,
                    "Note that doing so will lose any un-synchronized local changes."
                )?;
                Ok(())
            }
            Ok(e) => Err(e.into()),
            Err(e) => Err(e.into()),
        },
    }
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

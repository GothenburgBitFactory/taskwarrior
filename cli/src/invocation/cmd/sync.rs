use taskchampion::{server::Server, Replica};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    server: &mut Box<dyn Server>,
) -> anyhow::Result<()> {
    replica.sync(server)?;
    writeln!(w, "sync complete.")?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use tempfile::TempDir;

    #[test]
    fn test_add() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let server_dir = TempDir::new().unwrap();
        let mut server = test_server(&server_dir);

        // Note that the details of the actual sync are tested thoroughly in the taskchampion crate
        execute(&mut w, &mut replica, &mut server).unwrap();
        assert_eq!(&w.into_string(), "sync complete.\n")
    }
}

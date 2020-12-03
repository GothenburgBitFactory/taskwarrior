use failure::Fallible;
use taskchampion::{server::Server, Replica};

pub(crate) fn execute(replica: &mut Replica, server: &mut Box<dyn Server>) -> Fallible<()> {
    replica.sync(server)?;
    println!("sync complete.");
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::cmd::test::{test_replica, test_server};
    use tempdir::TempDir;

    #[test]
    fn test_add() {
        let mut replica = test_replica();
        let server_dir = TempDir::new("test").unwrap();
        let mut server = test_server(&server_dir);

        // this just has to not fail -- the details of the actual sync are
        // tested thoroughly in the taskchampion crate
        execute(&mut replica, &mut server).unwrap();
    }
}

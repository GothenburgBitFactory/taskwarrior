use std::io;
use taskchampion::{server, storage, Replica, ServerConfig};
use tempdir::TempDir;

pub(super) fn test_replica() -> Replica {
    let storage = storage::InMemoryStorage::new();
    Replica::new(Box::new(storage))
}

pub(super) fn test_server(dir: &TempDir) -> Box<dyn server::Server> {
    server::from_config(ServerConfig::Local {
        server_dir: dir.path().to_path_buf(),
    })
    .unwrap()
}

pub(super) struct TestWriter {
    data: Vec<u8>,
}

impl TestWriter {
    pub(super) fn into_string(self) -> String {
        String::from_utf8(self.data).unwrap()
    }
}

impl io::Write for TestWriter {
    fn write(&mut self, buf: &[u8]) -> io::Result<usize> {
        self.data.write(buf)
    }
    fn flush(&mut self) -> io::Result<()> {
        self.data.flush()
    }
}

impl termcolor::WriteColor for TestWriter {
    fn supports_color(&self) -> bool {
        false
    }
    fn set_color(&mut self, _spec: &termcolor::ColorSpec) -> io::Result<()> {
        Ok(())
    }
    fn reset(&mut self) -> io::Result<()> {
        Ok(())
    }
}

pub(super) fn test_writer() -> TestWriter {
    TestWriter { data: vec![] }
}

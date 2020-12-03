use taskchampion::{server, taskstorage, Replica, ServerConfig};
use tempdir::TempDir;

pub(super) fn test_replica() -> Replica {
    let storage = taskstorage::InMemoryStorage::new();
    Replica::new(Box::new(storage))
}

pub(super) fn test_server(dir: &TempDir) -> Box<dyn server::Server> {
    server::from_config(ServerConfig::Local {
        server_dir: dir.path().to_path_buf(),
    })
    .unwrap()
}

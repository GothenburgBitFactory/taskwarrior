use super::{InMemoryStorage, SqliteStorage, Storage};
use crate::errors::Result;
use std::path::PathBuf;

/// The configuration required for a replica's storage.
pub enum StorageConfig {
    /// Store the data on disk.  This is the common choice.
    OnDisk {
        /// Path containing the task DB.
        taskdb_dir: PathBuf,

        /// Create the DB if it does not already exist
        create_if_missing: bool,
    },
    /// Store the data in memory.  This is only useful for testing.
    InMemory,
}

impl StorageConfig {
    pub fn into_storage(self) -> Result<Box<dyn Storage>> {
        Ok(match self {
            StorageConfig::OnDisk {
                taskdb_dir,
                create_if_missing,
            } => Box::new(SqliteStorage::new(taskdb_dir, create_if_missing)?),
            StorageConfig::InMemory => Box::new(InMemoryStorage::new()),
        })
    }
}

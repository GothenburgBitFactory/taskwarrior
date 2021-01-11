use std::path::PathBuf;

/// The configuration required for a replica.  Use with [`crate::Replica::from_config`].
pub struct ReplicaConfig {
    /// Path containing the task DB.
    pub taskdb_dir: PathBuf,
}

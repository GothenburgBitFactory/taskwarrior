use std::path::PathBuf;
use uuid::Uuid;

/// The configuration required for a replica.  Use with [`crate::Replica::from_config`].
pub struct ReplicaConfig {
    /// Path containing the task DB.
    pub taskdb_dir: PathBuf,
}

/// The configuration for a replica's access to a sync server.  Use with
/// [`crate::server::from_config`].
pub enum ServerConfig {
    /// A local task database, for situations with a single replica.
    Local {
        /// Path containing the server's DB
        server_dir: PathBuf,
    },
    /// A remote taskchampion-sync-server instance
    Remote {
        /// Sync server "origin"; a URL with schema and hostname but no path or trailing `/`
        origin: String,

        /// Client ID to identify this replica to the server
        client_id: Uuid,
    },
}

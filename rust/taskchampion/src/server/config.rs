use super::types::Server;
use super::{LocalServer, RemoteServer};
use std::path::PathBuf;
use uuid::Uuid;

/// The configuration for a replica's access to a sync server.
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

        /// Client Key to identify and authenticate this replica to the server
        client_key: Uuid,

        /// Private encryption secret used to encrypt all data sent to the server.  This can
        /// be any suitably un-guessable string of bytes.
        encryption_secret: Vec<u8>,
    },
}

impl ServerConfig {
    /// Get a server based on this configuration
    pub fn into_server(self) -> anyhow::Result<Box<dyn Server>> {
        Ok(match self {
            ServerConfig::Local { server_dir } => Box::new(LocalServer::new(server_dir)?),
            ServerConfig::Remote {
                origin,
                client_key,
                encryption_secret,
            } => Box::new(RemoteServer::new(origin, client_key, encryption_secret)?),
        })
    }
}

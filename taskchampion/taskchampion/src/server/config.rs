use super::types::Server;
use crate::errors::Result;
#[cfg(feature = "server-gcp")]
use crate::server::cloud::gcp::GcpService;
#[cfg(feature = "cloud")]
use crate::server::cloud::CloudServer;
use crate::server::local::LocalServer;
#[cfg(feature = "server-sync")]
use crate::server::sync::SyncServer;
use std::path::PathBuf;
#[cfg(feature = "server-sync")]
use uuid::Uuid;

/// The configuration for a replica's access to a sync server.
pub enum ServerConfig {
    /// A local task database, for situations with a single replica.
    Local {
        /// Path containing the server's DB
        server_dir: PathBuf,
    },
    /// A remote taskchampion-sync-server instance
    #[cfg(feature = "server-sync")]
    Remote {
        /// Sync server "origin"; a URL with schema and hostname but no path or trailing `/`
        origin: String,

        /// Client ID to identify and authenticate this replica to the server
        client_id: Uuid,

        /// Private encryption secret used to encrypt all data sent to the server.  This can
        /// be any suitably un-guessable string of bytes.
        encryption_secret: Vec<u8>,
    },
    /// A remote taskchampion-sync-server instance
    #[cfg(feature = "server-gcp")]
    Gcp {
        /// Bucket in which to store the task data. This bucket must not be used for any other
        /// purpose.
        bucket: String,

        /// Private encryption secret used to encrypt all data sent to the server.  This can
        /// be any suitably un-guessable string of bytes.
        encryption_secret: Vec<u8>,
    },
}

impl ServerConfig {
    /// Get a server based on this configuration
    pub fn into_server(self) -> Result<Box<dyn Server>> {
        Ok(match self {
            ServerConfig::Local { server_dir } => Box::new(LocalServer::new(server_dir)?),
            #[cfg(feature = "server-sync")]
            ServerConfig::Remote {
                origin,
                client_id,
                encryption_secret,
            } => Box::new(SyncServer::new(origin, client_id, encryption_secret)?),
            #[cfg(feature = "server-gcp")]
            ServerConfig::Gcp {
                bucket,
                encryption_secret,
            } => Box::new(CloudServer::new(
                GcpService::new(bucket)?,
                encryption_secret,
            )?),
        })
    }
}

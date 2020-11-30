use crate::ServerConfig;
use failure::Fallible;

#[cfg(test)]
pub(crate) mod test;

mod local;
mod remote;
mod types;

pub use local::LocalServer;
pub use remote::RemoteServer;
pub use types::*;

/// Create a new server based on the given configuration.
pub fn from_config(config: ServerConfig) -> Fallible<Box<dyn Server>> {
    Ok(match config {
        ServerConfig::Local { server_dir } => Box::new(LocalServer::new(server_dir)?),
        ServerConfig::Remote { origin, client_id } => {
            Box::new(RemoteServer::new(origin, client_id))
        }
    })
}

use crate::server::SyncServer;
use std::sync::Arc;

pub(crate) mod add_version;
pub(crate) mod get_child_version;

/// The type containing a reference to the SyncServer object in the Actix state.
pub(crate) type ServerState = Arc<Box<dyn SyncServer>>;

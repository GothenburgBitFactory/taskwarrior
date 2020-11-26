use crate::server::SyncServer;
use std::sync::Arc;

pub(crate) mod add_version;
pub(crate) mod get_child_version;

/// The content-type for history segments (opaque blobs of bytes)
pub(crate) const HISTORY_SEGMENT_CONTENT_TYPE: &str =
    "application/vnd.taskchampion.history-segment";

/// The header names for version ID
pub(crate) const VERSION_ID_HEADER: &str = "X-Version-Id";

/// The header names for parent version ID
pub(crate) const PARENT_VERSION_ID_HEADER: &str = "X-Parent-Version-Id";

/// The type containing a reference to the SyncServer object in the Actix state.
pub(crate) type ServerState = Arc<Box<dyn SyncServer>>;

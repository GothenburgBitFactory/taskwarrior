use serde::{Deserialize, Serialize};
use taskchampion::Uuid;

pub(crate) type HistorySegment = Vec<u8>;
pub(crate) type ClientId = Uuid;
pub(crate) type VersionId = Uuid;

/// Response to get_child_version
#[derive(Serialize, Deserialize)]
pub(crate) struct GetVersionResult {
    pub(crate) version_id: Uuid,
    pub(crate) parent_version_id: Uuid,
    pub(crate) history_segment: HistorySegment,
}

/// Response to add_version
#[derive(Serialize, Deserialize)]
pub(crate) enum AddVersionResult {
    /// OK, version added with the given ID
    Ok(VersionId),
    /// Rejected; expected a version with the given parent version
    ExpectedParentVersion(VersionId),
}

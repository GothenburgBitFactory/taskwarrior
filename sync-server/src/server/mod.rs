use failure::Fallible;
use taskchampion::Uuid;

mod inmemory;

pub(crate) use inmemory::InMemorySyncServer;

/// The distinguished value for "no version"
pub const NO_VERSION_ID: VersionId = Uuid::nil();

pub(crate) type HistorySegment = Vec<u8>;
pub(crate) type ClientId = Uuid;
pub(crate) type VersionId = Uuid;

/// Response to get_child_version
#[derive(Clone)]
pub(crate) struct GetVersionResult {
    pub(crate) version_id: Uuid,
    pub(crate) parent_version_id: Uuid,
    pub(crate) history_segment: HistorySegment,
}

/// Response to add_version
#[derive(Clone)]
pub(crate) enum AddVersionResult {
    /// OK, version added with the given ID
    Ok(VersionId),
    /// Rejected; expected a version with the given parent version
    ExpectedParentVersion(VersionId),
}

pub(crate) trait SyncServer: Sync + Send {
    fn get_child_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
    ) -> Fallible<Option<GetVersionResult>>;

    fn add_version(
        &self,
        client_id: ClientId,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult>;
}

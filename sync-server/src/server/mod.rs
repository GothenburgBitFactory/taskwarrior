use failure::Fallible;
use taskchampion::Uuid;

pub(crate) type HistorySegment = Vec<u8>;
pub(crate) type ClientId = Uuid;
pub(crate) type VersionId = Uuid;

/// Response to get_child_version
pub(crate) struct GetVersionResult {
    pub(crate) version_id: Uuid,
    pub(crate) parent_version_id: Uuid,
    pub(crate) history_segment: HistorySegment,
}

/// Response to add_version
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

// TODO: temporary
/// A "null" sync server's implementation; HTTP API methods call through to methods on a single
/// instance of this type.
pub(crate) struct NullSyncServer {}

impl NullSyncServer {
    pub(crate) fn new() -> Self {
        Self {}
    }
}

impl SyncServer for NullSyncServer {
    fn get_child_version(
        &self,
        _client_id: ClientId,
        parent_version_id: VersionId,
    ) -> Fallible<Option<GetVersionResult>> {
        Ok(Some(GetVersionResult {
            version_id: Uuid::new_v4(),
            parent_version_id,
            history_segment: b"abcd".to_vec(),
        }))
    }

    fn add_version(
        &self,
        _client_id: ClientId,
        _parent_version_id: VersionId,
        _history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult> {
        //Ok(AddVersionResult::Ok(Uuid::new_v4()))
        Ok(AddVersionResult::ExpectedParentVersion(Uuid::new_v4()))
    }
}

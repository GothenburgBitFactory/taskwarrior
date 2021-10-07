use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, SnapshotUrgency, VersionId,
    NIL_VERSION_ID,
};
use std::collections::HashMap;
use uuid::Uuid;

struct Version {
    version_id: VersionId,
    parent_version_id: VersionId,
    history_segment: HistorySegment,
}

pub(crate) struct TestServer {
    latest_version_id: VersionId,
    // NOTE: indexed by parent_version_id!
    versions: HashMap<VersionId, Version>,
}

impl TestServer {
    /// A test server has no notion of clients, signatures, encryption, etc.
    pub fn new() -> TestServer {
        TestServer {
            latest_version_id: NIL_VERSION_ID,
            versions: HashMap::new(),
        }
    }
}

impl Server for TestServer {
    /// Add a new version.  If the given version number is incorrect, this responds with the
    /// appropriate version and expects the caller to try again.
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> anyhow::Result<(AddVersionResult, SnapshotUrgency)> {
        // no client lookup
        // no signature validation

        // check the parent_version_id for linearity
        if self.latest_version_id != NIL_VERSION_ID {
            if parent_version_id != self.latest_version_id {
                return Ok((
                    AddVersionResult::ExpectedParentVersion(self.latest_version_id),
                    SnapshotUrgency::None,
                ));
            }
        }

        // invent a new ID for this version
        let version_id = Uuid::new_v4();

        self.versions.insert(
            parent_version_id,
            Version {
                version_id,
                parent_version_id,
                history_segment,
            },
        );
        self.latest_version_id = version_id;

        Ok((AddVersionResult::Ok(version_id), SnapshotUrgency::None))
    }

    /// Get a vector of all versions after `since_version`
    fn get_child_version(
        &mut self,
        parent_version_id: VersionId,
    ) -> anyhow::Result<GetVersionResult> {
        if let Some(version) = self.versions.get(&parent_version_id) {
            Ok(GetVersionResult::Version {
                version_id: version.version_id,
                parent_version_id: version.parent_version_id,
                history_segment: version.history_segment.clone(),
            })
        } else {
            Ok(GetVersionResult::NoSuchVersion)
        }
    }
}

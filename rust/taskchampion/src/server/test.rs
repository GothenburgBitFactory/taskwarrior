use crate::server::{
    AddVersionResult, GetVersionResult, HistorySegment, Server, Snapshot, SnapshotUrgency,
    VersionId, NIL_VERSION_ID,
};
use std::collections::HashMap;
use std::sync::{Arc, Mutex};
use uuid::Uuid;

struct Version {
    version_id: VersionId,
    parent_version_id: VersionId,
    history_segment: HistorySegment,
}

/// TestServer implements the Server trait with a test implementation.
#[derive(Clone)]
pub(crate) struct TestServer(Arc<Mutex<Inner>>);

pub(crate) struct Inner {
    latest_version_id: VersionId,
    // NOTE: indexed by parent_version_id!
    versions: HashMap<VersionId, Version>,
    snapshot_urgency: SnapshotUrgency,
    snapshot: Option<(VersionId, Snapshot)>,
}

impl TestServer {
    /// A test server has no notion of clients, signatures, encryption, etc.
    pub(crate) fn new() -> TestServer {
        TestServer(Arc::new(Mutex::new(Inner {
            latest_version_id: NIL_VERSION_ID,
            versions: HashMap::new(),
            snapshot_urgency: SnapshotUrgency::None,
            snapshot: None,
        })))
    }
    // feel free to add any test utility functions here

    /// Get a boxed Server implementation referring to this TestServer
    pub(crate) fn server(&self) -> Box<dyn Server> {
        Box::new(self.clone())
    }

    pub(crate) fn set_snapshot_urgency(&self, urgency: SnapshotUrgency) {
        let mut inner = self.0.lock().unwrap();
        inner.snapshot_urgency = urgency;
    }

    /// Get the latest snapshot added to this server
    pub(crate) fn snapshot(&self) -> Option<(VersionId, Snapshot)> {
        let inner = self.0.lock().unwrap();
        inner.snapshot.as_ref().cloned()
    }

    /// Delete a version from storage
    pub(crate) fn delete_version(&mut self, parent_version_id: VersionId) {
        let mut inner = self.0.lock().unwrap();
        inner.versions.remove(&parent_version_id);
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
        let mut inner = self.0.lock().unwrap();

        // no client lookup
        // no signature validation

        // check the parent_version_id for linearity
        if inner.latest_version_id != NIL_VERSION_ID && parent_version_id != inner.latest_version_id
        {
            return Ok((
                AddVersionResult::ExpectedParentVersion(inner.latest_version_id),
                SnapshotUrgency::None,
            ));
        }

        // invent a new ID for this version
        let version_id = Uuid::new_v4();

        inner.versions.insert(
            parent_version_id,
            Version {
                version_id,
                parent_version_id,
                history_segment,
            },
        );
        inner.latest_version_id = version_id;

        // reply with the configured urgency and reset it to None
        let urgency = inner.snapshot_urgency;
        inner.snapshot_urgency = SnapshotUrgency::None;
        Ok((AddVersionResult::Ok(version_id), urgency))
    }

    /// Get a vector of all versions after `since_version`
    fn get_child_version(
        &mut self,
        parent_version_id: VersionId,
    ) -> anyhow::Result<GetVersionResult> {
        let inner = self.0.lock().unwrap();

        if let Some(version) = inner.versions.get(&parent_version_id) {
            Ok(GetVersionResult::Version {
                version_id: version.version_id,
                parent_version_id: version.parent_version_id,
                history_segment: version.history_segment.clone(),
            })
        } else {
            Ok(GetVersionResult::NoSuchVersion)
        }
    }

    fn add_snapshot(&mut self, version_id: VersionId, snapshot: Snapshot) -> anyhow::Result<()> {
        let mut inner = self.0.lock().unwrap();

        // test implementation -- does not perform any validation
        inner.snapshot = Some((version_id, snapshot));
        Ok(())
    }

    fn get_snapshot(&mut self) -> anyhow::Result<Option<(VersionId, Snapshot)>> {
        let inner = self.0.lock().unwrap();
        Ok(inner.snapshot.clone())
    }
}

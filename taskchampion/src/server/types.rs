use failure::Fallible;
use uuid::Uuid;

/// Versions are referred to with sha2 hashes.
pub type VersionId = Uuid;

/// The distinguished value for "no version"
pub const NO_VERSION_ID: VersionId = Uuid::nil();

/// A segment in the history of this task database, in the form of a sequence of operations.  This
/// data is pre-encoded, and from the protocol level appears as a sequence of bytes.
pub type HistorySegment = Vec<u8>;

/// VersionAdd is the response type from [`crate::server::Server::add_version`].
#[derive(Debug, PartialEq)]
pub enum AddVersionResult {
    /// OK, version added with the given ID
    Ok(VersionId),
    /// Rejected; expected a version with the given parent version
    ExpectedParentVersion(VersionId),
}

/// A version as downloaded from the server
#[derive(Debug, PartialEq)]
pub enum GetVersionResult {
    /// No such version exists
    NoSuchVersion,

    /// The requested version
    Version {
        version_id: VersionId,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    },
}

/// A value implementing this trait can act as a server against which a replica can sync.
pub trait Server {
    /// Add a new version.
    fn add_version(
        &mut self,
        parent_version_id: VersionId,
        history_segment: HistorySegment,
    ) -> Fallible<AddVersionResult>;

    /// Get the version with the given parent VersionId
    fn get_child_version(&mut self, parent_version_id: VersionId) -> Fallible<GetVersionResult>;
}

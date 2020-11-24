/// A Blob is a hunk of encoded data that is sent to the server.  The server does not interpret
/// this data at all.
pub type Blob = Vec<u8>;

/// VersionAdd is the response type from [`crate:server::Server::add_version`].
pub enum VersionAdd {
    /// OK, version added
    Ok,
    /// Rejected, must be based on the the given version
    ExpectedVersion(u64),
}

/// A value implementing this trait can act as a server against which a replica can sync.
pub trait Server {
    /// Get a vector of all versions after `since_version`
    fn get_versions(&self, username: &str, since_version: u64) -> Vec<Blob>;

    /// Add a new version.  If the given version number is incorrect, this responds with the
    /// appropriate version and expects the caller to try again.
    fn add_version(&mut self, username: &str, version: u64, blob: Blob) -> VersionAdd;

    fn add_snapshot(&mut self, username: &str, version: u64, blob: Blob);
}


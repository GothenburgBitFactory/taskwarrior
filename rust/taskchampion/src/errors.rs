use thiserror::Error;

#[derive(Debug, Error, Eq, PartialEq, Clone)]
#[non_exhaustive]
/// Errors returned from taskchampion operations
pub enum Error {
    /// A task-database-related error
    #[error("Task Database Error: {0}")]
    Database(String),
    /// An error specifically indicating that the local replica cannot
    /// be synchronized with the sever, due to being out of date or some
    /// other irrecoverable error.
    #[error("Local replica is out of sync with the server")]
    OutOfSync,
}

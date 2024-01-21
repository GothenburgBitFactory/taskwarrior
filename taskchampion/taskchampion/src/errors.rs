use std::io;
use thiserror::Error;

#[derive(Debug, Error)]
#[non_exhaustive]
/// Errors returned from taskchampion operations
pub enum Error {
    /// A server-related error
    #[error("Server Error: {0}")]
    Server(String),
    /// A task-database-related error
    #[error("Task Database Error: {0}")]
    Database(String),
    /// An error specifically indicating that the local replica cannot
    /// be synchronized with the sever, due to being out of date or some
    /// other irrecoverable error.
    #[error("Local replica is out of sync with the server")]
    OutOfSync,
    /// A usage error
    #[error("Usage Error: {0}")]
    Usage(String),
    /// A general error.
    #[error(transparent)]
    Other(#[from] anyhow::Error),
}

/// Convert private and third party errors into Error::Other.
macro_rules! other_error {
    ( $error:ty ) => {
        impl From<$error> for Error {
            fn from(err: $error) -> Self {
                Self::Other(err.into())
            }
        }
    };
}
#[cfg(feature = "server-sync")]
other_error!(ureq::Error);
other_error!(io::Error);
other_error!(serde_json::Error);
other_error!(rusqlite::Error);
other_error!(crate::storage::sqlite::SqliteError);
#[cfg(feature = "server-gcp")]
other_error!(google_cloud_storage::http::Error);
#[cfg(feature = "server-gcp")]
other_error!(google_cloud_storage::client::google_cloud_auth::error::Error);

pub type Result<T> = std::result::Result<T, Error>;

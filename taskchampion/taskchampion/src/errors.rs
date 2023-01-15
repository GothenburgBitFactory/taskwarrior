use std::io;
use thiserror::Error;

#[derive(Debug, Error)]
#[non_exhaustive]
/// Errors returned from taskchampion operations
pub enum Error {
    /// A crypto-related error
    #[error("Crypto Error: {0}")]
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
    #[error("User Error: {0}")]
    Usage(String),

    /// Error conversions.
    #[error(transparent)]
    Http(#[from] ureq::Error),
    #[error(transparent)]
    Io(#[from] io::Error),
    #[error(transparent)]
    Json(#[from] serde_json::Error),
    #[error(transparent)]
    Other(#[from] anyhow::Error),
    #[error("Third Party Sqlite Error")]
    Rusqlite(#[from] rusqlite::Error),
    #[error(transparent)]
    Sqlite(#[from] crate::storage::sqlite::SqliteError),
}

pub type Result<T> = std::result::Result<T, Error>;

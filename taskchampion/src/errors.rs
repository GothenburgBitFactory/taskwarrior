use thiserror::Error;

#[derive(Debug, Error, Eq, PartialEq, Clone)]
#[non_exhaustive]
/// Errors returned from taskchampion operations
pub enum Error {
    #[error("Task Database Error: {0}")]
    Database(String),
}

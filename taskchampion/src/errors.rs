use thiserror::Error;
#[derive(Debug, Error, Eq, PartialEq, Clone)]
pub enum Error {
    #[error("Task Database Error: {}", _0)]
    DBError(String),
}

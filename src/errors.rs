use failure::Fail;

#[derive(Debug, Fail, Eq, PartialEq, Clone)]
pub enum Error {
    #[fail(display = "Task Database Error: {}", _0)]
    DBError(String),

    #[fail(display = "Replica Error: {}", _0)]
    ReplicaError(String),
}

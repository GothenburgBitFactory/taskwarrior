use failure::Fail;

#[derive(Debug, Fail, Eq, PartialEq, Clone)]
pub enum Error {
    #[fail(display = "Task Database Error: {}", _0)]
    DBError(String),

    #[fail(display = "TDB2 Error: {}", _0)]
    TDB2Error(String),
}

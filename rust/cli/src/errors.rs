use taskchampion::Error as TcError;
use thiserror::Error;

#[derive(Debug, Error)]
pub enum Error {
    #[error("Command-Line Syntax Error: {0}")]
    Arguments(String),

    #[error(transparent)]
    TaskChampion(#[from] TcError),

    #[error(transparent)]
    Other(#[from] anyhow::Error),
}

impl Error {
    /// Construct a new command-line argument error
    pub(crate) fn for_arguments<S: ToString>(msg: S) -> Self {
        Error::Arguments(msg.to_string())
    }

    /// Determine the exit status for this error, as documented.
    pub fn exit_status(&self) -> i32 {
        match *self {
            Error::Arguments(_) => 3,
            _ => 1,
        }
    }
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Self {
        let err: anyhow::Error = err.into();
        Error::Other(err)
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use anyhow::anyhow;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_exit_status() {
        let mut err: Error;

        err = anyhow!("uhoh").into();
        assert_eq!(err.exit_status(), 1);

        err = Error::Arguments("uhoh".to_string());
        assert_eq!(err.exit_status(), 3);

        err = std::io::Error::last_os_error().into();
        assert_eq!(err.exit_status(), 1);

        err = TcError::Database("uhoh".to_string()).into();
        assert_eq!(err.exit_status(), 1);
    }
}

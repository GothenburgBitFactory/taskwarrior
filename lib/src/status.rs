pub use taskchampion::Status;

/// The status of a task, as defined by the task data model.
/// cbindgen:prefix-with-name
/// cbindgen:rename-all=ScreamingSnakeCase
#[repr(C)]
pub enum TCStatus {
    Pending,
    Completed,
    Deleted,
    /// Unknown signifies a status in the task DB that was not
    /// recognized.
    Unknown,
}

impl From<TCStatus> for Status {
    fn from(status: TCStatus) -> Status {
        match status {
            TCStatus::Pending => Status::Pending,
            TCStatus::Completed => Status::Completed,
            TCStatus::Deleted => Status::Deleted,
            TCStatus::Unknown => Status::Unknown("unknown".to_string()),
        }
    }
}

impl From<Status> for TCStatus {
    fn from(status: Status) -> TCStatus {
        match status {
            Status::Pending => TCStatus::Pending,
            Status::Completed => TCStatus::Completed,
            Status::Deleted => TCStatus::Deleted,
            Status::Unknown(_) => TCStatus::Unknown,
        }
    }
}

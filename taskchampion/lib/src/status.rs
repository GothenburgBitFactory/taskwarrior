pub use taskchampion::Status;

#[ffizz_header::item]
#[ffizz(order = 700)]
/// ***** TCStatus *****
///
/// The status of a task, as defined by the task data model.
///
/// ```c
/// #ifdef __cplusplus
/// typedef enum TCStatus : int32_t {
/// #else // __cplusplus
/// typedef int32_t TCStatus;
/// enum TCStatus {
/// #endif // __cplusplus
///   TC_STATUS_PENDING = 0,
///   TC_STATUS_COMPLETED = 1,
///   TC_STATUS_DELETED = 2,
///   TC_STATUS_RECURRING = 3,
///   // Unknown signifies a status in the task DB that was not
///   // recognized.
///   TC_STATUS_UNKNOWN = -1,
/// #ifdef __cplusplus
/// } TCStatus;
/// #else // __cplusplus
/// };
/// #endif // __cplusplus
/// ```
#[repr(i32)]
pub enum TCStatus {
    Pending = 0,
    Completed = 1,
    Deleted = 2,
    Recurring = 3,
    Unknown = -1,
}

impl From<TCStatus> for Status {
    fn from(status: TCStatus) -> Status {
        match status {
            TCStatus::Pending => Status::Pending,
            TCStatus::Completed => Status::Completed,
            TCStatus::Deleted => Status::Deleted,
            TCStatus::Recurring => Status::Recurring,
            _ => Status::Unknown(format!("unknown TCStatus {}", status as u32)),
        }
    }
}

impl From<Status> for TCStatus {
    fn from(status: Status) -> TCStatus {
        match status {
            Status::Pending => TCStatus::Pending,
            Status::Completed => TCStatus::Completed,
            Status::Deleted => TCStatus::Deleted,
            Status::Recurring => TCStatus::Recurring,
            Status::Unknown(_) => TCStatus::Unknown,
        }
    }
}

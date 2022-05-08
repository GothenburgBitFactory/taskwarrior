/// The status of a task, as defined by the task data model.
#[derive(Debug, PartialEq, Clone, strum_macros::Display)]
#[repr(C)]
pub enum Status {
    Pending,
    Completed,
    Deleted,
    /// Unknown signifies a status in the task DB that was not
    /// recognized.  This supports forward-compatibility if a
    /// new status is added.  Tasks with unknown status should
    /// be ignored (but not deleted).
    Unknown(String),
}

impl Status {
    /// Get a Status from the 1-character value in a TaskMap,
    /// defaulting to Pending
    pub(crate) fn from_taskmap(s: &str) -> Status {
        match s {
            "pending" => Status::Pending,
            "completed" => Status::Completed,
            "deleted" => Status::Deleted,
            v => Status::Unknown(v.to_string()),
        }
    }

    /// Get the 1-character value for this status to use in the TaskMap.
    pub(crate) fn to_taskmap(&self) -> &str {
        match self {
            Status::Pending => "pending",
            Status::Completed => "completed",
            Status::Deleted => "deleted",
            Status::Unknown(v) => v.as_ref(),
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn to_taskmap() {
        assert_eq!(Status::Pending.to_taskmap(), "pending");
        assert_eq!(Status::Completed.to_taskmap(), "completed");
        assert_eq!(Status::Deleted.to_taskmap(), "deleted");
        assert_eq!(Status::Unknown("wishful".into()).to_taskmap(), "wishful");
    }

    #[test]
    fn from_taskmap() {
        assert_eq!(Status::from_taskmap("pending"), Status::Pending);
        assert_eq!(Status::from_taskmap("completed"), Status::Completed);
        assert_eq!(Status::from_taskmap("deleted"), Status::Deleted);
        assert_eq!(
            Status::from_taskmap("something-else"),
            Status::Unknown("something-else".into())
        );
    }

    #[test]
    fn display() {
        assert_eq!(format!("{}", Status::Pending), "Pending");
        assert_eq!(format!("{}", Status::Completed), "Completed");
        assert_eq!(format!("{}", Status::Deleted), "Deleted");
        assert_eq!(format!("{}", Status::Unknown("wishful".into())), "Unknown");
    }
}

/// The status of a task.  The default status in "Pending".
#[derive(Debug, PartialEq, Clone)]
pub enum Status {
    Pending,
    Completed,
    Deleted,
}

impl Status {
    /// Get a Status from the 1-character value in a TaskMap,
    /// defaulting to Pending
    pub(crate) fn from_taskmap(s: &str) -> Status {
        match s {
            "P" => Status::Pending,
            "C" => Status::Completed,
            "D" => Status::Deleted,
            _ => Status::Pending,
        }
    }

    /// Get the 1-character value for this status to use in the TaskMap.
    pub(crate) fn to_taskmap(&self) -> &str {
        match self {
            Status::Pending => "P",
            Status::Completed => "C",
            Status::Deleted => "D",
        }
    }

    /// Get the full-name value for this status to use in the TaskMap.
    pub fn to_string(&self) -> &str {
        // TODO: should be impl Display
        match self {
            Status::Pending => "Pending",
            Status::Completed => "Completed",
            Status::Deleted => "Deleted",
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_status() {
        assert_eq!(Status::Pending.to_taskmap(), "P");
        assert_eq!(Status::Completed.to_taskmap(), "C");
        assert_eq!(Status::Deleted.to_taskmap(), "D");
        assert_eq!(Status::from_taskmap("P"), Status::Pending);
        assert_eq!(Status::from_taskmap("C"), Status::Completed);
        assert_eq!(Status::from_taskmap("D"), Status::Deleted);
    }
}

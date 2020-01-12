use chrono::prelude::*;
use std::collections::HashMap;
use std::convert::TryFrom;
use uuid::Uuid;

pub type Timestamp = DateTime<Utc>;

#[derive(Debug, PartialEq)]
pub enum Priority {
    L,
    M,
    H,
}

impl TryFrom<&str> for Priority {
    type Error = failure::Error;

    fn try_from(s: &str) -> Result<Self, Self::Error> {
        match s {
            "L" => Ok(Priority::L),
            "M" => Ok(Priority::M),
            "H" => Ok(Priority::H),
            _ => Err(format_err!("invalid status {}", s)),
        }
    }
}

impl AsRef<str> for Priority {
    fn as_ref(&self) -> &str {
        match self {
            Priority::L => "L",
            Priority::M => "M",
            Priority::H => "H",
        }
    }
}
#[derive(Debug, PartialEq)]
pub enum Status {
    Pending,
    Completed,
    Deleted,
    Recurring,
    Waiting,
}

impl TryFrom<&str> for Status {
    type Error = failure::Error;

    fn try_from(s: &str) -> Result<Self, Self::Error> {
        match s {
            "pending" => Ok(Status::Pending),
            "completed" => Ok(Status::Completed),
            "deleted" => Ok(Status::Deleted),
            "recurring" => Ok(Status::Recurring),
            "waiting" => Ok(Status::Waiting),
            _ => Err(format_err!("invalid status {}", s)),
        }
    }
}

impl AsRef<str> for Status {
    fn as_ref(&self) -> &str {
        match self {
            Status::Pending => "pending",
            Status::Completed => "completed",
            Status::Deleted => "deleted",
            Status::Recurring => "recurring",
            Status::Waiting => "waiting",
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct Annotation {
    pub entry: Timestamp,
    pub description: String,
}

/// A task, the fundamental business object of this tool.
///
/// This structure is based on https://taskwarrior.org/docs/design/task.html with the
/// exception that the uuid property is omitted.
#[derive(Debug, PartialEq)]
pub struct Task {
    pub status: Status,
    pub entry: Timestamp,
    pub description: String,
    pub start: Option<Timestamp>,
    pub end: Option<Timestamp>,
    pub due: Option<Timestamp>,
    pub until: Option<Timestamp>,
    pub wait: Option<Timestamp>,
    pub modified: Timestamp,
    pub scheduled: Option<Timestamp>,
    pub recur: Option<String>,
    pub mask: Option<String>,
    pub imask: Option<u64>,
    pub parent: Option<Uuid>,
    pub project: Option<String>,
    pub priority: Option<Priority>,
    pub depends: Vec<Uuid>,
    pub tags: Vec<String>,
    pub annotations: Vec<Annotation>,
    pub udas: HashMap<String, String>,
}

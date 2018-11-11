use std::collections::HashMap;
use uuid::Uuid;
use chrono::prelude::*;

pub type Timestamp = DateTime<Utc>;

#[derive(Debug, PartialEq)]
pub enum Priority {
    L,
    M,
    H,
}

#[derive(Debug, PartialEq)]
pub enum Status {
    Pending,
    Completed,
    Deleted,
    Recurring,
    Waiting,
}

#[derive(Debug, PartialEq)]
pub struct Annotation {
    pub entry: Timestamp,
    pub description: String,
}

/// A task, the fundamental business object of this tool.
///
/// This structure is based on https://taskwarrior.org/docs/design/task.html
#[derive(Debug)]
pub struct Task {
    pub status: Status,
    pub uuid: Uuid,
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

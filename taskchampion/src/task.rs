use crate::replica::Replica;
use crate::taskstorage::TaskMap;
use chrono::prelude::*;
use failure::Fallible;
use uuid::Uuid;

pub type Timestamp = DateTime<Utc>;

/// The priority of a task
#[derive(Debug, PartialEq)]
pub enum Priority {
    /// Low
    L,
    /// Medium
    M,
    /// High
    H,
}

#[allow(dead_code)]
impl Priority {
    /// Get a Priority from the 1-character value in a TaskMap,
    /// defaulting to M
    pub(crate) fn from_taskmap(s: &str) -> Priority {
        match s {
            "L" => Priority::L,
            "M" => Priority::M,
            "H" => Priority::H,
            _ => Priority::M,
        }
    }

    /// Get the 1-character value for this priority to use in the TaskMap.
    pub(crate) fn to_taskmap(&self) -> &str {
        match self {
            Priority::L => "L",
            Priority::M => "M",
            Priority::H => "H",
        }
    }
}

/// The status of a task.  The default status in "Pending".
#[derive(Debug, PartialEq)]
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

#[derive(Debug, PartialEq)]
pub struct Annotation {
    pub entry: Timestamp,
    pub description: String,
}

/// A task, as publicly exposed by this crate.
///
/// Note that Task objects represent a snapshot of the task at a moment in time, and are not
/// protected by the atomicity of the backend storage.  Concurrent modifications are safe,
/// but a Task that is cached for more than a few seconds may cause the user to see stale
/// data.  Fetch, use, and drop Tasks quickly.
///
/// This struct contains only getters for various values on the task. The `into_mut` method returns
/// a TaskMut which can be used to modify the task.
#[derive(Debug, PartialEq)]
pub struct Task {
    uuid: Uuid,
    taskmap: TaskMap,
}

/// A mutable task, with setter methods.  Most methods are simple setters and not further
/// described.  Calling a setter will update the Replica, as well as the included Task.
pub struct TaskMut<'r> {
    task: Task,
    replica: &'r mut Replica,
    updated_modified: bool,
}

impl Task {
    pub(crate) fn new(uuid: Uuid, taskmap: TaskMap) -> Task {
        Task { uuid, taskmap }
    }

    pub fn get_uuid(&self) -> &Uuid {
        &self.uuid
    }

    /// Prepare to mutate this task, requiring a mutable Replica
    /// in order to update the data it contains.
    pub fn into_mut(self, replica: &mut Replica) -> TaskMut {
        TaskMut {
            task: self,
            replica: replica,
            updated_modified: false,
        }
    }

    pub fn get_status(&self) -> Status {
        self.taskmap
            .get("status")
            .map(|s| Status::from_taskmap(s))
            .unwrap_or(Status::Pending)
    }

    pub fn get_description(&self) -> &str {
        self.taskmap
            .get("description")
            .map(|s| s.as_ref())
            .unwrap_or("")
    }

    pub fn get_modified(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp("modified")
    }

    // -- utility functions

    pub fn get_timestamp(&self, property: &str) -> Option<DateTime<Utc>> {
        if let Some(ts) = self.taskmap.get(property) {
            if let Ok(ts) = ts.parse() {
                return Some(Utc.timestamp(ts, 0));
            }
            // if the value does not parse as an integer, default to None
        }
        None
    }
}

impl<'r> TaskMut<'r> {
    /// Get the immutable version of this object.  Note that TaskMut [`std::ops::Deref`]s to
    /// [`crate::task::Task`], so all of that struct's getter methods can be used on TaskMut.
    pub fn into_immut(self) -> Task {
        self.task
    }

    /// Set the task's status.  This also adds the task to the working set if the
    /// new status puts it in that set.
    pub fn set_status(&mut self, status: Status) -> Fallible<()> {
        if status == Status::Pending {
            let uuid = self.uuid.clone();
            self.replica.add_to_working_set(&uuid)?;
        }
        self.set_string("status", Some(String::from(status.to_taskmap())))
    }

    pub fn set_description(&mut self, description: String) -> Fallible<()> {
        self.set_string("description", Some(description))
    }

    pub fn set_modified(&mut self, modified: DateTime<Utc>) -> Fallible<()> {
        self.set_timestamp("modified", Some(modified))
    }

    // -- utility functions

    fn lastmod(&mut self) -> Fallible<()> {
        if !self.updated_modified {
            let now = format!("{}", Utc::now().timestamp());
            self.replica
                .update_task(self.task.uuid.clone(), "modified", Some(now.clone()))?;
            self.task.taskmap.insert(String::from("modified"), now);
            self.updated_modified = true;
        }
        Ok(())
    }

    fn set_string(&mut self, property: &str, value: Option<String>) -> Fallible<()> {
        self.lastmod()?;
        self.replica
            .update_task(self.task.uuid.clone(), property, value.as_ref())?;

        if let Some(v) = value {
            self.task.taskmap.insert(property.to_string(), v);
        } else {
            self.task.taskmap.remove(property);
        }
        Ok(())
    }

    fn set_timestamp(&mut self, property: &str, value: Option<DateTime<Utc>>) -> Fallible<()> {
        self.lastmod()?;
        self.replica.update_task(
            self.task.uuid.clone(),
            property,
            value.map(|v| format!("{}", v.timestamp())),
        )
    }
}

impl<'r> std::ops::Deref for TaskMut<'r> {
    type Target = Task;

    fn deref(&self) -> &Self::Target {
        &self.task
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_priority() {
        assert_eq!(Priority::L.to_taskmap(), "L");
        assert_eq!(Priority::M.to_taskmap(), "M");
        assert_eq!(Priority::H.to_taskmap(), "H");
        assert_eq!(Priority::from_taskmap("L"), Priority::L);
        assert_eq!(Priority::from_taskmap("M"), Priority::M);
        assert_eq!(Priority::from_taskmap("H"), Priority::H);
    }

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

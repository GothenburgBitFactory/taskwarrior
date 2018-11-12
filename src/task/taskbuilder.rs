use std::collections::HashMap;
use chrono::prelude::*;
use std::str;
use uuid::Uuid;
use task::{Task, Priority, Status, Timestamp, Annotation};

#[derive(Default)]
pub struct TaskBuilder {
    status: Option<Status>,
    uuid: Option<Uuid>,
    entry: Option<Timestamp>,
    description: Option<String>,
    start: Option<Timestamp>,
    end: Option<Timestamp>,
    due: Option<Timestamp>,
    until: Option<Timestamp>,
    wait: Option<Timestamp>,
    modified: Option<Timestamp>,
    scheduled: Option<Timestamp>,
    recur: Option<String>,
    mask: Option<String>,
    imask: Option<u64>,
    parent: Option<Uuid>,
    project: Option<String>,
    priority: Option<Priority>,
    depends: Vec<Uuid>,
    tags: Vec<String>,
    annotations: Vec<Annotation>,
    udas: HashMap<String, String>,
}

/// Parse an "integer", allowing for occasional integers with trailing decimal zeroes
fn parse_int<T>(value: &str) -> Result<T, <T as str::FromStr>::Err>
where
    T: str::FromStr,
{
    // some integers are rendered with following decimal zeroes
    if let Some(i) = value.find('.') {
        let mut nonzero = false;
        for c in value[i + 1..].chars() {
            if c != '0' {
                nonzero = true;
                break;
            }
        }
        if !nonzero {
            return value[..i].parse();
        }
    }
    value.parse()
}

/// Parse a status into a Status enum value
fn parse_status(value: &str) -> Result<Status, String> {
    match value {
        "pending" => Ok(Status::Pending),
        "completed" => Ok(Status::Completed),
        "deleted" => Ok(Status::Deleted),
        "recurring" => Ok(Status::Recurring),
        "waiting" => Ok(Status::Waiting),
        _ => Err(format!("invalid status {}", value)),
    }
}

/// Parse "L", "M", "H" into the Priority enum

fn parse_priority(value: &str) -> Result<Priority, String> {
    match value {
        "L" => Ok(Priority::L),
        "M" => Ok(Priority::M),
        "H" => Ok(Priority::H),
        _ => Err(format!("invalid priority {}", value)),
    }
}

/// Parse a UNIX timestamp into a UTC DateTime
fn parse_timestamp(value: &str) -> Result<Timestamp, <i64 as str::FromStr>::Err> {
    Ok(Utc.timestamp(parse_int::<i64>(value)?, 0))
}

/// Parse depends, as a list of ,-separated UUIDs
fn parse_depends(value: &str) -> Result<Vec<Uuid>, uuid::parser::ParseError> {
    value.split(',').map(|s| Uuid::parse_str(s)).collect()
}

/// Parse tags, as a list of ,-separated strings
fn parse_tags(value: &str) -> Vec<String> {
    value.split(',').map(|s| s.to_string()).collect()
}

impl TaskBuilder {
    pub fn new() -> Self {
        Default::default()
    }

    pub fn set(mut self, name: &str, value: String) -> Self {
        const ANNOTATION_PREFIX: &str = "annotation_";
        if name.starts_with(ANNOTATION_PREFIX) {
            let entry = parse_timestamp(&name[ANNOTATION_PREFIX.len()..]).unwrap();
            // TODO: sort by entry time
            self.annotations.push(Annotation {
                entry,
                description: value.to_string(),
            });
            return self;
        }
        match name {
            "status" => self.status = Some(parse_status(&value).unwrap()),
            "uuid" => self.uuid = Some(Uuid::parse_str(&value).unwrap()),
            "entry" => self.entry = Some(parse_timestamp(&value).unwrap()),
            "description" => self.description = Some(value),
            "start" => self.start = Some(parse_timestamp(&value).unwrap()),
            "end" => self.end = Some(parse_timestamp(&value).unwrap()),
            "due" => self.due = Some(parse_timestamp(&value).unwrap()),
            "until" => self.until = Some(parse_timestamp(&value).unwrap()),
            "wait" => self.wait = Some(parse_timestamp(&value).unwrap()),
            "modified" => self.modified = Some(parse_timestamp(&value).unwrap()),
            "scheduled" => self.scheduled = Some(parse_timestamp(&value).unwrap()),
            "recur" => self.recur = Some(value),
            "mask" => self.mask = Some(value),
            "imask" => self.imask = Some(parse_int::<u64>(&value).unwrap()),
            "parent" => self.uuid = Some(Uuid::parse_str(&value).unwrap()),
            "project" => self.project = Some(value),
            "priority" => self.priority = Some(parse_priority(&value).unwrap()),
            "depends" => self.depends = parse_depends(&value).unwrap(),
            "tags" => self.tags = parse_tags(&value),
            _ => {
                self.udas.insert(name.to_string(), value);
            }
        }
        self
    }

    pub fn finish(self) -> Task {
        Task {
            status: self.status.unwrap(),
            uuid: self.uuid.unwrap(),
            description: self.description.unwrap(),
            entry: self.entry.unwrap(),
            start: self.start,
            end: self.end,
            due: self.due,
            until: self.until,
            wait: self.wait,
            modified: self.modified.unwrap(),
            scheduled: self.scheduled,
            recur: self.recur,
            mask: self.mask,
            imask: self.imask,
            parent: self.parent,
            project: self.project,
            priority: self.priority,
            depends: self.depends,
            tags: self.tags,
            annotations: self.annotations,
            udas: self.udas,
        }

        // TODO: check validity per https://taskwarrior.org/docs/design/task.html
    }
}

#[cfg(test)]
mod test {
    use super::{parse_int, parse_depends};
    use uuid::Uuid;

    #[test]
    fn test_parse_int() {
        assert_eq!(parse_int::<u8>("123").unwrap(), 123u8);
        assert_eq!(parse_int::<u32>("123000000").unwrap(), 123000000u32);
        assert_eq!(parse_int::<i32>("-123000000").unwrap(), -123000000i32);
    }

    #[test]
    fn test_parse_int_decimals() {
        assert_eq!(parse_int::<u8>("123.00").unwrap(), 123u8);
        assert_eq!(parse_int::<u32>("123.0000").unwrap(), 123u32);
        assert_eq!(parse_int::<i32>("-123.").unwrap(), -123i32);
    }

    #[test]
    fn test_parse_depends() {
        let u1 = "123e4567-e89b-12d3-a456-426655440000";
        let u2 = "123e4567-e89b-12d3-a456-999999990000";
        assert_eq!(
            parse_depends(u1).unwrap(),
            vec![Uuid::parse_str(u1).unwrap()]
        );
        assert_eq!(
            parse_depends(&format!("{},{}", u1, u2)).unwrap(),
            vec![Uuid::parse_str(u1).unwrap(), Uuid::parse_str(u2).unwrap()]
        );
    }
}

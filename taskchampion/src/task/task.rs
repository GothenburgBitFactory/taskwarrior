use super::tag::{SyntheticTag, TagInner};
use super::{Annotation, Status, Tag, Timestamp};
use crate::depmap::DependencyMap;
use crate::replica::Replica;
use crate::storage::TaskMap;
use chrono::prelude::*;
use log::trace;
use std::convert::AsRef;
use std::convert::TryInto;
use std::rc::Rc;
use std::str::FromStr;
use uuid::Uuid;

/* The Task and TaskMut classes wrap the underlying [`TaskMap`], which is a simple key/value map.
 * They provide semantic meaning to that TaskMap according to the TaskChampion data model.  For
 * example, [`get_status`](Task::get_status) and [`set_status`](TaskMut::set_status) translate from
 * strings in the TaskMap to [`Status`].
 *
 * The same approach applies for more complex data such as dependencies or annotations.  Users of
 * this API should only need the [`get_taskmap`](Task::get_taskmap) method for debugging purposes,
 * and should never need to make changes to the TaskMap directly.
 */

/// A task, as publicly exposed by this crate.
///
/// Note that Task objects represent a snapshot of the task at a moment in time, and are not
/// protected by the atomicity of the backend storage.  Concurrent modifications are safe,
/// but a Task that is cached for more than a few seconds may cause the user to see stale
/// data.  Fetch, use, and drop Tasks quickly.
///
/// This struct contains only getters for various values on the task. The
/// [`into_mut`](Task::into_mut) method
/// returns a TaskMut which can be used to modify the task.
#[derive(Debug, Clone)]
pub struct Task {
    uuid: Uuid,
    taskmap: TaskMap,
    depmap: Rc<DependencyMap>,
}

impl PartialEq for Task {
    fn eq(&self, other: &Task) -> bool {
        // compare only the taskmap and uuid; depmap is just present for reference
        self.uuid == other.uuid && self.taskmap == other.taskmap
    }
}

/// A mutable task, with setter methods.
///
/// Most methods are simple setters and not further described.  Calling a setter will update the
/// referenced Replica, as well as the included Task, immediately.
///
/// The [`Task`] methods are available on [`TaskMut`] via [`Deref`](std::ops::Deref).
pub struct TaskMut<'r> {
    task: Task,
    replica: &'r mut Replica,
    updated_modified: bool,
}

/// An enum containing all of the key names defined in the data model, with the exception
/// of the properties containing data (`tag_..`, etc.)
#[derive(strum_macros::AsRefStr, strum_macros::EnumString)]
#[strum(serialize_all = "kebab-case")]
enum Prop {
    Description,
    Modified,
    Start,
    Status,
    Priority,
    Wait,
    End,
    Entry,
}

#[allow(clippy::ptr_arg)]
fn uda_string_to_tuple(key: &str) -> (&str, &str) {
    let mut iter = key.splitn(2, '.');
    let first = iter.next().unwrap();
    let second = iter.next();
    if let Some(second) = second {
        (first, second)
    } else {
        ("", first)
    }
}

fn uda_tuple_to_string(namespace: impl AsRef<str>, key: impl AsRef<str>) -> String {
    let namespace = namespace.as_ref();
    let key = key.as_ref();
    if namespace.is_empty() {
        key.into()
    } else {
        format!("{}.{}", namespace, key)
    }
}

impl Task {
    pub(crate) fn new(uuid: Uuid, taskmap: TaskMap, depmap: Rc<DependencyMap>) -> Task {
        Task {
            uuid,
            taskmap,
            depmap,
        }
    }

    pub fn get_uuid(&self) -> Uuid {
        self.uuid
    }

    pub fn get_taskmap(&self) -> &TaskMap {
        &self.taskmap
    }

    /// Prepare to mutate this task, requiring a mutable Replica
    /// in order to update the data it contains.
    pub fn into_mut(self, replica: &mut Replica) -> TaskMut {
        TaskMut {
            task: self,
            replica,
            updated_modified: false,
        }
    }

    pub fn get_status(&self) -> Status {
        self.taskmap
            .get(Prop::Status.as_ref())
            .map(|s| Status::from_taskmap(s))
            .unwrap_or(Status::Pending)
    }

    pub fn get_description(&self) -> &str {
        self.taskmap
            .get(Prop::Description.as_ref())
            .map(|s| s.as_ref())
            .unwrap_or("")
    }

    pub fn get_entry(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp(Prop::Entry.as_ref())
    }

    pub fn get_priority(&self) -> &str {
        self.taskmap
            .get(Prop::Priority.as_ref())
            .map(|s| s.as_ref())
            .unwrap_or("")
    }

    /// Get the wait time.  If this value is set, it will be returned, even
    /// if it is in the past.
    pub fn get_wait(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp(Prop::Wait.as_ref())
    }

    /// Determine whether this task is waiting now.
    pub fn is_waiting(&self) -> bool {
        if let Some(ts) = self.get_wait() {
            return ts > Utc::now();
        }
        false
    }

    /// Determine whether this task is active -- that is, that it has been started
    /// and not stopped.
    pub fn is_active(&self) -> bool {
        self.taskmap.contains_key(Prop::Start.as_ref())
    }

    /// Determine whether this task is blocked -- that is, has at least one unresolved dependency.
    pub fn is_blocked(&self) -> bool {
        self.depmap.dependencies(self.uuid).next().is_some()
    }

    /// Determine whether this task is blocking -- that is, has at least one unresolved dependent.
    pub fn is_blocking(&self) -> bool {
        self.depmap.dependents(self.uuid).next().is_some()
    }

    /// Determine whether a given synthetic tag is present on this task.  All other
    /// synthetic tag calculations are based on this one.
    fn has_synthetic_tag(&self, synth: &SyntheticTag) -> bool {
        match synth {
            SyntheticTag::Waiting => self.is_waiting(),
            SyntheticTag::Active => self.is_active(),
            SyntheticTag::Pending => self.get_status() == Status::Pending,
            SyntheticTag::Completed => self.get_status() == Status::Completed,
            SyntheticTag::Deleted => self.get_status() == Status::Deleted,
            SyntheticTag::Blocked => self.is_blocked(),
            SyntheticTag::Unblocked => !self.is_blocked(),
            SyntheticTag::Blocking => self.is_blocking(),
        }
    }

    /// Check if this task has the given tag
    pub fn has_tag(&self, tag: &Tag) -> bool {
        match tag.inner() {
            TagInner::User(s) => self.taskmap.contains_key(&format!("tag_{}", s)),
            TagInner::Synthetic(st) => self.has_synthetic_tag(st),
        }
    }

    /// Iterate over the task's tags
    pub fn get_tags(&self) -> impl Iterator<Item = Tag> + '_ {
        use strum::IntoEnumIterator;

        self.taskmap
            .iter()
            .filter_map(|(k, _)| {
                if let Some(tag) = k.strip_prefix("tag_") {
                    if let Ok(tag) = tag.try_into() {
                        return Some(tag);
                    }
                    // note that invalid "tag_*" are ignored
                }
                None
            })
            .chain(
                SyntheticTag::iter()
                    .filter(move |st| self.has_synthetic_tag(st))
                    .map(|st| Tag::from_inner(TagInner::Synthetic(st))),
            )
    }

    /// Iterate over the task's annotations, in arbitrary order.
    pub fn get_annotations(&self) -> impl Iterator<Item = Annotation> + '_ {
        self.taskmap.iter().filter_map(|(k, v)| {
            if let Some(ts) = k.strip_prefix("annotation_") {
                if let Ok(ts) = ts.parse::<i64>() {
                    return Some(Annotation {
                        entry: Utc.timestamp(ts, 0),
                        description: v.to_owned(),
                    });
                }
                // note that invalid "annotation_*" are ignored
            }
            None
        })
    }

    /// Get the named user defined attributes (UDA).  This will return None
    /// for any key defined in the Task data model, regardless of whether
    /// it is set or not.
    pub fn get_uda(&self, namespace: &str, key: &str) -> Option<&str> {
        self.get_legacy_uda(uda_tuple_to_string(namespace, key).as_ref())
    }

    /// Get the user defined attributes (UDAs) of this task, in arbitrary order.  Each key is split
    /// on the first `.` character.  Legacy keys that do not contain `.` are represented as `("",
    /// key)`.
    pub fn get_udas(&self) -> impl Iterator<Item = ((&str, &str), &str)> + '_ {
        self.taskmap
            .iter()
            .filter(|(k, _)| !Task::is_known_key(k))
            .map(|(k, v)| (uda_string_to_tuple(k), v.as_ref()))
    }

    /// Get the named user defined attribute (UDA) in a legacy format.  This will return None for
    /// any key defined in the Task data model, regardless of whether it is set or not.
    pub fn get_legacy_uda(&self, key: &str) -> Option<&str> {
        if Task::is_known_key(key) {
            return None;
        }
        self.taskmap.get(key).map(|s| s.as_ref())
    }

    /// Like `get_udas`, but returning each UDA key as a single string.
    pub fn get_legacy_udas(&self) -> impl Iterator<Item = (&str, &str)> + '_ {
        self.taskmap
            .iter()
            .filter(|(p, _)| !Task::is_known_key(p))
            .map(|(p, v)| (p.as_ref(), v.as_ref()))
    }

    /// Get the modification time for this task.
    pub fn get_modified(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp(Prop::Modified.as_ref())
    }

    /// Get the UUIDs of tasks on which this task depends.
    ///
    /// This includes all dependencies, regardless of their status.  In fact, it may include
    /// dependencies that do not exist.
    pub fn get_dependencies(&self) -> impl Iterator<Item = Uuid> + '_ {
        self.taskmap.iter().filter_map(|(p, _)| {
            if let Some(dep_str) = p.strip_prefix("dep_") {
                if let Ok(u) = Uuid::parse_str(dep_str) {
                    return Some(u);
                }
                // (un-parseable dep_.. properties are ignored)
            }
            None
        })
    }

    // -- utility functions

    fn is_known_key(key: &str) -> bool {
        Prop::from_str(key).is_ok()
            || key.starts_with("tag_")
            || key.starts_with("annotation_")
            || key.starts_with("dep_")
    }

    fn get_timestamp(&self, property: &str) -> Option<DateTime<Utc>> {
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
    /// Get the immutable version of this object, ending the exclusive reference to the Replica.
    pub fn into_immut(self) -> Task {
        self.task
    }

    /// Set the task's status.  This also adds the task to the working set if the
    /// new status puts it in that set.
    pub fn set_status(&mut self, status: Status) -> anyhow::Result<()> {
        match status {
            Status::Pending => {
                // clear "end" when a task becomes "pending"
                if self.taskmap.contains_key(Prop::End.as_ref()) {
                    self.set_timestamp(Prop::End.as_ref(), None)?;
                }
                let uuid = self.uuid;
                self.replica.add_to_working_set(uuid)?;
            }
            Status::Completed | Status::Deleted => {
                // set "end" when a task is deleted or completed
                if !self.taskmap.contains_key(Prop::End.as_ref()) {
                    self.set_timestamp(Prop::End.as_ref(), Some(Utc::now()))?;
                }
            }
            _ => {}
        }
        self.set_string(
            Prop::Status.as_ref(),
            Some(String::from(status.to_taskmap())),
        )
    }

    pub fn set_description(&mut self, description: String) -> anyhow::Result<()> {
        self.set_string(Prop::Description.as_ref(), Some(description))
    }

    pub fn set_priority(&mut self, priority: String) -> anyhow::Result<()> {
        self.set_string(Prop::Priority.as_ref(), Some(priority))
    }

    pub fn set_entry(&mut self, entry: Option<DateTime<Utc>>) -> anyhow::Result<()> {
        self.set_timestamp(Prop::Entry.as_ref(), entry)
    }

    pub fn set_wait(&mut self, wait: Option<DateTime<Utc>>) -> anyhow::Result<()> {
        self.set_timestamp(Prop::Wait.as_ref(), wait)
    }

    pub fn set_modified(&mut self, modified: DateTime<Utc>) -> anyhow::Result<()> {
        self.set_timestamp(Prop::Modified.as_ref(), Some(modified))
    }

    /// Start the task by creating "start": "<timestamp>", if the task is not already
    /// active.
    pub fn start(&mut self) -> anyhow::Result<()> {
        if self.is_active() {
            return Ok(());
        }
        self.set_timestamp(Prop::Start.as_ref(), Some(Utc::now()))
    }

    /// Stop the task by removing the `start` key
    pub fn stop(&mut self) -> anyhow::Result<()> {
        self.set_timestamp(Prop::Start.as_ref(), None)
    }

    /// Mark this task as complete
    pub fn done(&mut self) -> anyhow::Result<()> {
        self.set_status(Status::Completed)
    }

    /// Mark this task as deleted.
    ///
    /// Note that this does not delete the task.  It merely marks the task as
    /// deleted.
    pub fn delete(&mut self) -> anyhow::Result<()> {
        self.set_status(Status::Deleted)
    }

    /// Add a tag to this task.  Does nothing if the tag is already present.
    pub fn add_tag(&mut self, tag: &Tag) -> anyhow::Result<()> {
        if tag.is_synthetic() {
            anyhow::bail!("Synthetic tags cannot be modified");
        }
        self.set_string(format!("tag_{}", tag), Some("".to_owned()))
    }

    /// Remove a tag from this task.  Does nothing if the tag is not present.
    pub fn remove_tag(&mut self, tag: &Tag) -> anyhow::Result<()> {
        if tag.is_synthetic() {
            anyhow::bail!("Synthetic tags cannot be modified");
        }
        self.set_string(format!("tag_{}", tag), None)
    }

    /// Add a new annotation.  Note that annotations with the same entry time
    /// will overwrite one another.
    pub fn add_annotation(&mut self, ann: Annotation) -> anyhow::Result<()> {
        self.set_string(
            format!("annotation_{}", ann.entry.timestamp()),
            Some(ann.description),
        )
    }

    /// Remove an annotation, based on its entry time.
    pub fn remove_annotation(&mut self, entry: Timestamp) -> anyhow::Result<()> {
        self.set_string(format!("annotation_{}", entry.timestamp()), None)
    }

    /// Set a user-defined attribute (UDA).  This will fail if the key is defined by the data
    /// model.
    pub fn set_uda(
        &mut self,
        namespace: impl AsRef<str>,
        key: impl AsRef<str>,
        value: impl Into<String>,
    ) -> anyhow::Result<()> {
        let key = uda_tuple_to_string(namespace, key);
        self.set_legacy_uda(key, value)
    }

    /// Remove a user-defined attribute (UDA).  This will fail if the key is defined by the data
    /// model.
    pub fn remove_uda(
        &mut self,
        namespace: impl AsRef<str>,
        key: impl AsRef<str>,
    ) -> anyhow::Result<()> {
        let key = uda_tuple_to_string(namespace, key);
        self.remove_legacy_uda(key)
    }

    /// Set a user-defined attribute (UDA), where the key is a legacy key.
    pub fn set_legacy_uda(
        &mut self,
        key: impl Into<String>,
        value: impl Into<String>,
    ) -> anyhow::Result<()> {
        let key = key.into();
        if Task::is_known_key(&key) {
            anyhow::bail!(
                "Property name {} as special meaning in a task and cannot be used as a UDA",
                key
            );
        }
        self.set_string(key, Some(value.into()))
    }

    /// Remove a user-defined attribute (UDA), where the key is a legacy key.
    pub fn remove_legacy_uda(&mut self, key: impl Into<String>) -> anyhow::Result<()> {
        let key = key.into();
        if Task::is_known_key(&key) {
            anyhow::bail!(
                "Property name {} as special meaning in a task and cannot be used as a UDA",
                key
            );
        }
        self.set_string(key, None)
    }

    /// Add a dependency.
    pub fn add_dependency(&mut self, dep: Uuid) -> anyhow::Result<()> {
        let key = format!("dep_{}", dep);
        self.set_string(key, Some("".to_string()))
    }

    /// Remove a dependency.
    pub fn remove_dependency(&mut self, dep: Uuid) -> anyhow::Result<()> {
        let key = format!("dep_{}", dep);
        self.set_string(key, None)
    }

    // -- utility functions

    fn update_modified(&mut self) -> anyhow::Result<()> {
        if !self.updated_modified {
            let now = format!("{}", Utc::now().timestamp());
            trace!("task {}: set property modified={:?}", self.task.uuid, now);
            self.task.taskmap =
                self.replica
                    .update_task(self.task.uuid, Prop::Modified.as_ref(), Some(now))?;
            self.updated_modified = true;
        }
        Ok(())
    }

    fn set_string<S: Into<String>>(
        &mut self,
        property: S,
        value: Option<String>,
    ) -> anyhow::Result<()> {
        let property = property.into();
        // updated the modified timestamp unless we are setting it explicitly
        if &property != "modified" {
            self.update_modified()?;
        }

        if let Some(ref v) = value {
            trace!("task {}: set property {}={:?}", self.task.uuid, property, v);
        } else {
            trace!("task {}: remove property {}", self.task.uuid, property);
        }

        self.task.taskmap = self
            .replica
            .update_task(self.task.uuid, &property, value.as_ref())?;

        Ok(())
    }

    fn set_timestamp(
        &mut self,
        property: &str,
        value: Option<DateTime<Utc>>,
    ) -> anyhow::Result<()> {
        self.set_string(property, value.map(|v| v.timestamp().to_string()))
    }

    /// Used by tests to ensure that updates are properly written
    #[cfg(test)]
    fn reload(&mut self) -> anyhow::Result<()> {
        let uuid = self.uuid;
        let task = self.replica.get_task(uuid)?.unwrap();
        self.task.taskmap = task.taskmap;
        Ok(())
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
    use pretty_assertions::assert_eq;
    use std::collections::HashSet;

    fn dm() -> Rc<DependencyMap> {
        Rc::new(DependencyMap::new())
    }

    fn with_mut_task<F: FnOnce(TaskMut)>(f: F) {
        let mut replica = Replica::new_inmemory();
        let task = replica.new_task(Status::Pending, "test".into()).unwrap();
        let task = task.into_mut(&mut replica);
        f(task)
    }

    /// Create a user tag, without checking its validity
    fn utag(name: &'static str) -> Tag {
        Tag::from_inner(TagInner::User(name.into()))
    }

    /// Create a synthetic tag
    fn stag(synth: SyntheticTag) -> Tag {
        Tag::from_inner(TagInner::Synthetic(synth))
    }

    #[test]
    fn test_is_active_never_started() {
        let task = Task::new(Uuid::new_v4(), TaskMap::new(), dm());
        assert!(!task.is_active());
    }

    #[test]
    fn test_is_active_active() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("start"), String::from("1234"))]
                .drain(..)
                .collect(),
            dm(),
        );

        assert!(task.is_active());
    }

    #[test]
    fn test_is_active_inactive() {
        let task = Task::new(Uuid::new_v4(), Default::default(), dm());
        assert!(!task.is_active());
    }

    #[test]
    fn test_entry_not_set() {
        let task = Task::new(Uuid::new_v4(), TaskMap::new(), dm());
        assert_eq!(task.get_entry(), None);
    }

    #[test]
    fn test_entry_set() {
        let ts = Utc.ymd(1980, 1, 1).and_hms(0, 0, 0);
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("entry"), format!("{}", ts.timestamp()))]
                .drain(..)
                .collect(),
            dm(),
        );
        assert_eq!(task.get_entry(), Some(ts));
    }

    #[test]
    fn test_wait_not_set() {
        let task = Task::new(Uuid::new_v4(), TaskMap::new(), dm());

        assert!(!task.is_waiting());
        assert_eq!(task.get_wait(), None);
    }

    #[test]
    fn test_wait_in_past() {
        let ts = Utc.ymd(1970, 1, 1).and_hms(0, 0, 0);
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("wait"), format!("{}", ts.timestamp()))]
                .drain(..)
                .collect(),
            dm(),
        );

        assert!(!task.is_waiting());
        assert_eq!(task.get_wait(), Some(ts));
    }

    #[test]
    fn test_wait_in_future() {
        let ts = Utc.ymd(3000, 1, 1).and_hms(0, 0, 0);
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("wait"), format!("{}", ts.timestamp()))]
                .drain(..)
                .collect(),
            dm(),
        );

        assert!(task.is_waiting());
        assert_eq!(task.get_wait(), Some(ts));
    }

    #[test]
    fn test_has_tag() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (String::from("tag_abc"), String::from("")),
                (String::from("start"), String::from("1234")),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        assert!(task.has_tag(&utag("abc")));
        assert!(!task.has_tag(&utag("def")));
        assert!(task.has_tag(&stag(SyntheticTag::Active)));
        assert!(task.has_tag(&stag(SyntheticTag::Pending)));
        assert!(!task.has_tag(&stag(SyntheticTag::Waiting)));
    }

    #[test]
    fn test_get_tags() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (String::from("tag_abc"), String::from("")),
                (String::from("tag_def"), String::from("")),
                // set `wait` so the synthetic tag WAITING is present
                (String::from("wait"), String::from("33158909732")),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        let tags: HashSet<_> = task.get_tags().collect();
        let exp = set![
            utag("abc"),
            utag("def"),
            stag(SyntheticTag::Pending),
            stag(SyntheticTag::Waiting),
            stag(SyntheticTag::Unblocked),
        ];
        assert_eq!(tags, exp);
    }

    #[test]
    fn test_get_tags_invalid_tags() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (String::from("tag_ok"), String::from("")),
                (String::from("tag_"), String::from("")),
                (String::from("tag_123"), String::from("")),
                (String::from("tag_a!!"), String::from("")),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        // only "ok" is OK
        let tags: HashSet<_> = task.get_tags().collect();
        assert_eq!(
            tags,
            set![
                utag("ok"),
                stag(SyntheticTag::Pending),
                stag(SyntheticTag::Unblocked)
            ]
        );
    }

    #[test]
    fn test_get_priority_default() {
        let task = Task::new(Uuid::new_v4(), TaskMap::new(), dm());
        assert_eq!(task.get_priority(), "");
    }

    #[test]
    fn test_get_annotations() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (
                    String::from("annotation_1635301873"),
                    String::from("left message"),
                ),
                (
                    String::from("annotation_1635301883"),
                    String::from("left another message"),
                ),
                (String::from("annotation_"), String::from("invalid")),
                (String::from("annotation_abcde"), String::from("invalid")),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        let mut anns: Vec<_> = task.get_annotations().collect();
        anns.sort();
        assert_eq!(
            anns,
            vec![
                Annotation {
                    entry: Utc.timestamp(1635301873, 0),
                    description: "left message".into()
                },
                Annotation {
                    entry: Utc.timestamp(1635301883, 0),
                    description: "left another message".into()
                }
            ]
        );
    }

    #[test]
    fn test_add_annotation() {
        with_mut_task(|mut task| {
            task.add_annotation(Annotation {
                entry: Utc.timestamp(1635301900, 0),
                description: "right message".into(),
            })
            .unwrap();
            let k = "annotation_1635301900";
            assert_eq!(task.taskmap[k], "right message".to_owned());
            task.reload().unwrap();
            assert_eq!(task.taskmap[k], "right message".to_owned());
            // adding with same time overwrites..
            task.add_annotation(Annotation {
                entry: Utc.timestamp(1635301900, 0),
                description: "right message 2".into(),
            })
            .unwrap();
            assert_eq!(task.taskmap[k], "right message 2".to_owned());
        });
    }

    #[test]
    fn test_remove_annotation() {
        with_mut_task(|mut task| {
            task.set_string("annotation_1635301873", Some("left message".into()))
                .unwrap();
            task.set_string("annotation_1635301883", Some("left another message".into()))
                .unwrap();

            task.remove_annotation(Utc.timestamp(1635301873, 0))
                .unwrap();

            task.reload().unwrap();

            let mut anns: Vec<_> = task.get_annotations().collect();
            anns.sort();
            assert_eq!(
                anns,
                vec![Annotation {
                    entry: Utc.timestamp(1635301883, 0),
                    description: "left another message".into()
                }]
            );
        });
    }

    #[test]
    fn test_set_get_priority() {
        with_mut_task(|mut task| {
            assert_eq!(task.get_priority(), "");
            task.set_priority("H".into()).unwrap();
            assert_eq!(task.get_priority(), "H");
        });
    }

    #[test]
    fn test_set_status_pending() {
        with_mut_task(|mut task| {
            task.done().unwrap();

            task.set_status(Status::Pending).unwrap();
            assert_eq!(task.get_status(), Status::Pending);
            assert!(!task.taskmap.contains_key("end"));
            assert!(task.has_tag(&stag(SyntheticTag::Pending)));
            assert!(!task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_set_status_completed() {
        with_mut_task(|mut task| {
            task.set_status(Status::Completed).unwrap();
            assert_eq!(task.get_status(), Status::Completed);
            assert!(task.taskmap.contains_key("end"));
            assert!(!task.has_tag(&stag(SyntheticTag::Pending)));
            assert!(task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_set_status_deleted() {
        with_mut_task(|mut task| {
            task.set_status(Status::Deleted).unwrap();
            assert_eq!(task.get_status(), Status::Deleted);
            assert!(task.taskmap.contains_key("end"));
            assert!(!task.has_tag(&stag(SyntheticTag::Pending)));
            assert!(!task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_start() {
        with_mut_task(|mut task| {
            task.start().unwrap();
            assert!(task.taskmap.contains_key("start"));

            task.reload().unwrap();
            assert!(task.taskmap.contains_key("start"));

            // second start doesn't change anything..
            task.start().unwrap();
            assert!(task.taskmap.contains_key("start"));

            task.reload().unwrap();
            assert!(task.taskmap.contains_key("start"));
        });
    }

    #[test]
    fn test_stop() {
        with_mut_task(|mut task| {
            task.start().unwrap();
            task.stop().unwrap();
            assert!(!task.taskmap.contains_key("start"));

            task.reload().unwrap();
            assert!(!task.taskmap.contains_key("start"));

            // redundant call does nothing..
            task.stop().unwrap();
            assert!(!task.taskmap.contains_key("start"));

            task.reload().unwrap();
            assert!(!task.taskmap.contains_key("start"));
        });
    }

    #[test]
    fn test_done() {
        with_mut_task(|mut task| {
            task.done().unwrap();
            assert_eq!(task.get_status(), Status::Completed);
            assert!(task.taskmap.contains_key("end"));
            assert!(task.has_tag(&stag(SyntheticTag::Completed)));

            // redundant call does nothing..
            task.done().unwrap();
            assert_eq!(task.get_status(), Status::Completed);
            assert!(task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_delete() {
        with_mut_task(|mut task| {
            task.delete().unwrap();
            assert_eq!(task.get_status(), Status::Deleted);
            assert!(task.taskmap.contains_key("end"));
            assert!(!task.has_tag(&stag(SyntheticTag::Completed)));

            // redundant call does nothing..
            task.delete().unwrap();
            assert_eq!(task.get_status(), Status::Deleted);
            assert!(!task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_add_tags() {
        with_mut_task(|mut task| {
            task.add_tag(&utag("abc")).unwrap();
            assert!(task.taskmap.contains_key("tag_abc"));
            task.reload().unwrap();
            assert!(task.taskmap.contains_key("tag_abc"));
            // redundant add has no effect..
            task.add_tag(&utag("abc")).unwrap();
            assert!(task.taskmap.contains_key("tag_abc"));
        });
    }

    #[test]
    fn test_remove_tags() {
        with_mut_task(|mut task| {
            task.add_tag(&utag("abc")).unwrap();
            task.reload().unwrap();
            assert!(task.taskmap.contains_key("tag_abc"));

            task.remove_tag(&utag("abc")).unwrap();
            assert!(!task.taskmap.contains_key("tag_abc"));
            // redundant remove has no effect..
            task.remove_tag(&utag("abc")).unwrap();
            assert!(!task.taskmap.contains_key("tag_abc"));
        });
    }

    #[test]
    fn test_get_udas() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                ("description".into(), "not a uda".into()),
                ("modified".into(), "not a uda".into()),
                ("start".into(), "not a uda".into()),
                ("status".into(), "not a uda".into()),
                ("wait".into(), "not a uda".into()),
                ("start".into(), "not a uda".into()),
                ("tag_abc".into(), "not a uda".into()),
                ("dep_1234".into(), "not a uda".into()),
                ("annotation_1234".into(), "not a uda".into()),
                ("githubid".into(), "123".into()),
                ("jira.url".into(), "h://x".into()),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        let mut udas: Vec<_> = task.get_udas().collect();
        udas.sort_unstable();
        assert_eq!(
            udas,
            vec![(("", "githubid"), "123"), (("jira", "url"), "h://x")]
        );
    }

    #[test]
    fn test_get_uda() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                ("description".into(), "not a uda".into()),
                ("githubid".into(), "123".into()),
                ("jira.url".into(), "h://x".into()),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        assert_eq!(task.get_uda("", "description"), None); // invalid UDA
        assert_eq!(task.get_uda("", "githubid"), Some("123"));
        assert_eq!(task.get_uda("jira", "url"), Some("h://x"));
        assert_eq!(task.get_uda("bugzilla", "url"), None);
    }

    #[test]
    fn test_get_legacy_uda() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                ("description".into(), "not a uda".into()),
                ("dep_1234".into(), "not a uda".into()),
                ("githubid".into(), "123".into()),
                ("jira.url".into(), "h://x".into()),
            ]
            .drain(..)
            .collect(),
            dm(),
        );

        assert_eq!(task.get_legacy_uda("description"), None); // invalid UDA
        assert_eq!(task.get_legacy_uda("dep_1234"), None); // invalid UDA
        assert_eq!(task.get_legacy_uda("githubid"), Some("123"));
        assert_eq!(task.get_legacy_uda("jira.url"), Some("h://x"));
        assert_eq!(task.get_legacy_uda("bugzilla.url"), None);
    }

    #[test]
    fn test_set_uda() {
        with_mut_task(|mut task| {
            task.set_uda("jira", "url", "h://y").unwrap();
            let udas: Vec<_> = task.get_udas().collect();
            assert_eq!(udas, vec![(("jira", "url"), "h://y")]);

            task.set_uda("", "jiraid", "TW-1234").unwrap();

            let mut udas: Vec<_> = task.get_udas().collect();
            udas.sort_unstable();
            assert_eq!(
                udas,
                vec![(("", "jiraid"), "TW-1234"), (("jira", "url"), "h://y")]
            );
        })
    }

    #[test]
    fn test_set_legacy_uda() {
        with_mut_task(|mut task| {
            task.set_legacy_uda("jira.url", "h://y").unwrap();
            let udas: Vec<_> = task.get_udas().collect();
            assert_eq!(udas, vec![(("jira", "url"), "h://y")]);

            task.set_legacy_uda("jiraid", "TW-1234").unwrap();

            let mut udas: Vec<_> = task.get_udas().collect();
            udas.sort_unstable();
            assert_eq!(
                udas,
                vec![(("", "jiraid"), "TW-1234"), (("jira", "url"), "h://y")]
            );
        })
    }

    #[test]
    fn test_set_uda_invalid() {
        with_mut_task(|mut task| {
            assert!(task.set_uda("", "modified", "123").is_err());
            assert!(task.set_uda("", "tag_abc", "123").is_err());
            assert!(task.set_legacy_uda("modified", "123").is_err());
            assert!(task.set_legacy_uda("tag_abc", "123").is_err());
        })
    }

    #[test]
    fn test_remove_uda() {
        with_mut_task(|mut task| {
            task.set_string("github.id", Some("123".into())).unwrap();
            task.remove_uda("github", "id").unwrap();

            let udas: Vec<_> = task.get_udas().collect();
            assert_eq!(udas, vec![]);
        })
    }

    #[test]
    fn test_remove_legacy_uda() {
        with_mut_task(|mut task| {
            task.set_string("githubid", Some("123".into())).unwrap();
            task.remove_legacy_uda("githubid").unwrap();

            let udas: Vec<_> = task.get_udas().collect();
            assert_eq!(udas, vec![]);
        })
    }

    #[test]
    fn test_remove_uda_invalid() {
        with_mut_task(|mut task| {
            assert!(task.remove_uda("", "modified").is_err());
            assert!(task.remove_uda("", "tag_abc").is_err());
            assert!(task.remove_legacy_uda("modified").is_err());
            assert!(task.remove_legacy_uda("tag_abc").is_err());
        })
    }

    #[test]
    fn test_dependencies() {
        with_mut_task(|mut task| {
            assert_eq!(task.get_dependencies().collect::<Vec<_>>(), vec![]);
            let dep1 = Uuid::new_v4();
            let dep2 = Uuid::new_v4();

            task.add_dependency(dep1).unwrap();
            assert_eq!(task.get_dependencies().collect::<Vec<_>>(), vec![dep1]);

            task.add_dependency(dep1).unwrap(); // add twice is ok
            task.add_dependency(dep2).unwrap();
            let deps = task.get_dependencies().collect::<Vec<_>>();
            assert!(deps.contains(&dep1));
            assert!(deps.contains(&dep2));

            task.remove_dependency(dep1).unwrap();
            assert_eq!(task.get_dependencies().collect::<Vec<_>>(), vec![dep2]);
        })
    }

    #[test]
    fn dependencies_tags() {
        let mut rep = Replica::new_inmemory();
        let uuid1;
        let uuid2;
        {
            let t1 = rep.new_task(Status::Pending, "1".into()).unwrap();
            uuid1 = t1.get_uuid();
            let t2 = rep.new_task(Status::Pending, "2".into()).unwrap();
            uuid2 = t2.get_uuid();

            let mut t1 = t1.into_mut(&mut rep);
            t1.add_dependency(t2.get_uuid()).unwrap();
        }

        // force-refresh depmap
        rep.dependency_map(true).unwrap();

        let t1 = rep.get_task(uuid1).unwrap().unwrap();
        let t2 = rep.get_task(uuid2).unwrap().unwrap();
        assert!(t1.has_tag(&stag(SyntheticTag::Blocked)));
        assert!(!t1.has_tag(&stag(SyntheticTag::Unblocked)));
        assert!(!t1.has_tag(&stag(SyntheticTag::Blocking)));
        assert!(!t2.has_tag(&stag(SyntheticTag::Blocked)));
        assert!(t2.has_tag(&stag(SyntheticTag::Unblocked)));
        assert!(t2.has_tag(&stag(SyntheticTag::Blocking)));
    }
}

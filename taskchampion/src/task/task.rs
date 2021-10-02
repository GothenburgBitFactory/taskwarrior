use super::tag::{SyntheticTag, TagInner};
use super::{Status, Tag};
use crate::replica::Replica;
use crate::storage::TaskMap;
use chrono::prelude::*;
use log::trace;
use std::convert::AsRef;
use std::convert::TryInto;
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
#[derive(Debug, Clone, PartialEq)]
pub struct Task {
    uuid: Uuid,
    taskmap: TaskMap,
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

impl Task {
    pub(crate) fn new(uuid: Uuid, taskmap: TaskMap) -> Task {
        Task { uuid, taskmap }
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

    /// Get the wait time.  If this value is set, it will be returned, even
    /// if it is in the past.
    pub fn get_wait(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp("wait")
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
        self.taskmap
            .iter()
            .any(|(k, v)| k.starts_with("start.") && v.is_empty())
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
        }
    }

    /// Check if this task has the given tag
    pub fn has_tag(&self, tag: &Tag) -> bool {
        match tag.inner() {
            TagInner::User(s) => self.taskmap.contains_key(&format!("tag.{}", s)),
            TagInner::Synthetic(st) => self.has_synthetic_tag(st),
        }
    }

    /// Iterate over the task's tags
    pub fn get_tags(&self) -> impl Iterator<Item = Tag> + '_ {
        use strum::IntoEnumIterator;

        self.taskmap
            .iter()
            .filter_map(|(k, _)| {
                if let Some(tag) = k.strip_prefix("tag.") {
                    if let Ok(tag) = tag.try_into() {
                        return Some(tag);
                    }
                    // note that invalid "tag.*" are ignored
                }
                None
            })
            .chain(
                SyntheticTag::iter()
                    .filter(move |st| self.has_synthetic_tag(st))
                    .map(|st| Tag::from_inner(TagInner::Synthetic(st))),
            )
    }

    pub fn get_modified(&self) -> Option<DateTime<Utc>> {
        self.get_timestamp("modified")
    }

    // -- utility functions

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
        if status == Status::Pending {
            let uuid = self.uuid;
            self.replica.add_to_working_set(uuid)?;
        }
        self.set_string("status", Some(String::from(status.to_taskmap())))
    }

    pub fn set_description(&mut self, description: String) -> anyhow::Result<()> {
        self.set_string("description", Some(description))
    }

    pub fn set_wait(&mut self, wait: Option<DateTime<Utc>>) -> anyhow::Result<()> {
        self.set_timestamp("wait", wait)
    }

    pub fn set_modified(&mut self, modified: DateTime<Utc>) -> anyhow::Result<()> {
        self.set_timestamp("modified", Some(modified))
    }

    /// Start the task by creating "start.<timestamp": "", if the task is not already
    /// active.
    pub fn start(&mut self) -> anyhow::Result<()> {
        if self.is_active() {
            return Ok(());
        }
        let k = format!("start.{}", Utc::now().timestamp());
        self.set_string(k, Some(String::from("")))
    }

    /// Stop the task by adding the current timestamp to all un-resolved "start.<timestamp>" keys.
    pub fn stop(&mut self) -> anyhow::Result<()> {
        let keys = self
            .taskmap
            .iter()
            .filter(|(k, v)| k.starts_with("start.") && v.is_empty())
            .map(|(k, _)| k)
            .cloned()
            .collect::<Vec<_>>();
        let now = Utc::now();
        for key in keys {
            println!("{}", key);
            self.set_timestamp(&key, Some(now))?;
        }
        Ok(())
    }

    /// Mark this task as complete
    pub fn done(&mut self) -> anyhow::Result<()> {
        self.set_status(Status::Completed)
    }

    /// Add a tag to this task.  Does nothing if the tag is already present.
    pub fn add_tag(&mut self, tag: &Tag) -> anyhow::Result<()> {
        if tag.is_synthetic() {
            anyhow::bail!("Synthetic tags cannot be modified");
        }
        self.set_string(format!("tag.{}", tag), Some("".to_owned()))
    }

    /// Remove a tag from this task.  Does nothing if the tag is not present.
    pub fn remove_tag(&mut self, tag: &Tag) -> anyhow::Result<()> {
        if tag.is_synthetic() {
            anyhow::bail!("Synthetic tags cannot be modified");
        }
        self.set_string(format!("tag.{}", tag), None)
    }

    // -- utility functions

    fn lastmod(&mut self) -> anyhow::Result<()> {
        if !self.updated_modified {
            let now = format!("{}", Utc::now().timestamp());
            self.replica
                .update_task(self.task.uuid, "modified", Some(now.clone()))?;
            trace!("task {}: set property modified={:?}", self.task.uuid, now);
            self.task.taskmap.insert(String::from("modified"), now);
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
        self.lastmod()?;
        self.replica
            .update_task(self.task.uuid, &property, value.as_ref())?;

        if let Some(v) = value {
            trace!("task {}: set property {}={:?}", self.task.uuid, property, v);
            self.task.taskmap.insert(property, v);
        } else {
            trace!("task {}: remove property {}", self.task.uuid, property);
            self.task.taskmap.remove(&property);
        }
        Ok(())
    }

    fn set_timestamp(
        &mut self,
        property: &str,
        value: Option<DateTime<Utc>>,
    ) -> anyhow::Result<()> {
        self.lastmod()?;
        if let Some(value) = value {
            let ts = format!("{}", value.timestamp());
            self.replica
                .update_task(self.task.uuid, property, Some(ts.clone()))?;
            self.task.taskmap.insert(property.to_string(), ts);
        } else {
            self.replica
                .update_task::<_, &str>(self.task.uuid, property, None)?;
            self.task.taskmap.remove(property);
        }
        Ok(())
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
        let task = Task::new(Uuid::new_v4(), TaskMap::new());
        assert!(!task.is_active());
    }

    #[test]
    fn test_is_active() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("start.1234"), String::from(""))]
                .drain(..)
                .collect(),
        );

        assert!(task.is_active());
    }

    #[test]
    fn test_is_active_stopped() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![(String::from("start.1234"), String::from("1235"))]
                .drain(..)
                .collect(),
        );

        assert!(!task.is_active());
    }

    #[test]
    fn test_wait_not_set() {
        let task = Task::new(Uuid::new_v4(), TaskMap::new());

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
        );
        dbg!(&task);

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
        );

        assert!(task.is_waiting());
        assert_eq!(task.get_wait(), Some(ts));
    }

    #[test]
    fn test_has_tag() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (String::from("tag.abc"), String::from("")),
                (String::from("start.1234"), String::from("")),
            ]
            .drain(..)
            .collect(),
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
                (String::from("tag.abc"), String::from("")),
                (String::from("tag.def"), String::from("")),
                // set `wait` so the synthetic tag WAITING is present
                (String::from("wait"), String::from("33158909732")),
            ]
            .drain(..)
            .collect(),
        );

        let mut tags: Vec<_> = task.get_tags().collect();
        tags.sort();
        let mut exp = vec![
            utag("abc"),
            utag("def"),
            stag(SyntheticTag::Pending),
            stag(SyntheticTag::Waiting),
        ];
        exp.sort();
        assert_eq!(tags, exp);
    }

    #[test]
    fn test_get_tags_invalid_tags() {
        let task = Task::new(
            Uuid::new_v4(),
            vec![
                (String::from("tag.ok"), String::from("")),
                (String::from("tag."), String::from("")),
                (String::from("tag.123"), String::from("")),
                (String::from("tag.a!!"), String::from("")),
            ]
            .drain(..)
            .collect(),
        );

        // only "ok" is OK
        let tags: Vec<_> = task.get_tags().collect();
        assert_eq!(tags, vec![utag("ok"), stag(SyntheticTag::Pending)]);
    }

    fn count_taskmap(task: &TaskMut, f: fn(&(&String, &String)) -> bool) -> usize {
        task.taskmap.iter().filter(f).count()
    }

    #[test]
    fn test_start() {
        with_mut_task(|mut task| {
            task.start().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                1
            );
            task.reload().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                1
            );

            // second start doesn't change anything..
            task.start().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                1
            );
            task.reload().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                1
            );
        });
    }

    #[test]
    fn test_stop() {
        with_mut_task(|mut task| {
            task.start().unwrap();
            task.stop().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                0
            );
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && !v.is_empty()),
                1
            );
            task.reload().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                0
            );
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && !v.is_empty()),
                1
            );
        });
    }

    #[test]
    fn test_done() {
        with_mut_task(|mut task| {
            task.done().unwrap();
            assert_eq!(task.get_status(), Status::Completed);
            assert!(task.has_tag(&stag(SyntheticTag::Completed)));

            // redundant call does nothing..
            task.done().unwrap();
            assert_eq!(task.get_status(), Status::Completed);
            assert!(task.has_tag(&stag(SyntheticTag::Completed)));
        });
    }

    #[test]
    fn test_stop_multiple() {
        with_mut_task(|mut task| {
            // simulate a task that has (through the synchronization process) been started twice
            task.task
                .taskmap
                .insert(String::from("start.1234"), String::from(""));
            task.task
                .taskmap
                .insert(String::from("start.5678"), String::from(""));

            task.stop().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                0
            );
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && !v.is_empty()),
                2
            );
            task.reload().unwrap();
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && v.is_empty()),
                0
            );
            assert_eq!(
                count_taskmap(&task, |(k, v)| k.starts_with("start.") && !v.is_empty()),
                2
            );
        });
    }

    #[test]
    fn test_add_tags() {
        with_mut_task(|mut task| {
            task.add_tag(&utag("abc")).unwrap();
            assert!(task.taskmap.contains_key("tag.abc"));
            task.reload().unwrap();
            assert!(task.taskmap.contains_key("tag.abc"));
            // redundant add has no effect..
            task.add_tag(&utag("abc")).unwrap();
            assert!(task.taskmap.contains_key("tag.abc"));
        });
    }

    #[test]
    fn test_remove_tags() {
        with_mut_task(|mut task| {
            task.add_tag(&utag("abc")).unwrap();
            task.reload().unwrap();
            assert!(task.taskmap.contains_key("tag.abc"));

            task.remove_tag(&utag("abc")).unwrap();
            assert!(!task.taskmap.contains_key("tag.abc"));
            // redundant remove has no effect..
            task.remove_tag(&utag("abc")).unwrap();
            assert!(!task.taskmap.contains_key("tag.abc"));
        });
    }
}

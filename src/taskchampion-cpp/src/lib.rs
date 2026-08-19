use cxx::CxxString;
use std::path::PathBuf;
use std::pin::Pin;
use taskchampion as tc;
use taskchampion::SqliteStorage;

// All Taskchampion FFI is contained in this module, due to issues with cxx and multiple modules
// such as https://github.com/dtolnay/cxx/issues/1323.

/// FFI interface for TaskChampion.
///
/// This loosely follows the TaskChampion API defined at
/// https://docs.rs/taskchampion/latest/taskchampion/, with adjustments made as necessary to
/// accomodate cxx's limitations. Consult that documentation for full descriptions of the types and
/// methods.
///
/// This interface is an internal implementation detail of Taskwarrior and may change at any time.
#[cxx::bridge(namespace = "tc")]
mod ffi {
    // --- Uuid

    #[derive(Debug, Eq, PartialEq, Clone, Copy)]
    struct Uuid {
        v: [u8; 16],
    }

    extern "Rust" {
        /// Generate a new, random Uuid.
        fn uuid_v4() -> Uuid;

        /// Parse the given string as a Uuid, panicking if it is not valid.
        fn uuid_from_string(uuid: Pin<&CxxString>) -> Uuid;

        /// Convert the given Uuid to a string.
        fn to_string(self: &Uuid) -> String;

        /// Check whether this is the "nil" Uuid, used as a sentinel value.
        fn is_nil(self: &Uuid) -> bool;
    }

    // --- Operation and Operations

    extern "Rust" {
        type Operation;

        /// Check if this is a Create operation.
        fn is_create(&self) -> bool;

        /// Check if this is a Update operation.
        fn is_update(&self) -> bool;

        /// Check if this is a Delete operation.
        fn is_delete(&self) -> bool;

        /// Check if this is an UndoPoint operation.
        fn is_undo_point(&self) -> bool;

        /// Get the operation's uuid.
        ///
        /// Only valid for create, update, and delete operations.
        fn get_uuid(&self) -> Uuid;

        /// Get the `old_task` for this update operation.
        ///
        /// Only valid for delete operations.
        fn get_old_task(&self) -> Vec<PropValuePair>;

        /// Get the `property` for this update operation.
        ///
        /// Only valid for update operations.
        fn get_property(&self, property_out: Pin<&mut CxxString>);

        /// Get the `value` for this update operation, returning false if the
        /// `value` field is None.
        ///
        /// Only valid for update operations.
        fn get_value(&self, value_out: Pin<&mut CxxString>) -> bool;

        /// Get the `old_value` for this update operation, returning false if the
        /// `old_value` field is None.
        ///
        /// Only valid for update operations.
        fn get_old_value(&self, old_value_out: Pin<&mut CxxString>) -> bool;

        /// Get the `timestamp` for this update operation.
        ///
        /// Only valid for update operations.
        fn get_timestamp(&self) -> i64;

        /// Create a new vector of operations. It's also fine to construct a
        /// `rust::Vec<tc::Operation>` directly.
        fn new_operations() -> Vec<Operation>;

        /// Add an UndoPoint operation to the vector of operations. All other
        /// operation types should be added via `TaskData`.
        fn add_undo_point(ops: &mut Vec<Operation>);
    }

    // --- Replica

    extern "Rust" {
        type Replica;

        /// Create a new replica stored on-disk.
        fn new_replica_on_disk(
            taskdb_dir: String,
            create_if_missing: bool,
            read_write: bool,
        ) -> Result<Box<Replica>>;

        /// Commit the given operations to the replica.
        fn commit_operations(&mut self, ops: Vec<Operation>) -> Result<()>;

        /// Commit the reverse of the given operations.
        fn commit_reversed_operations(&mut self, ops: Vec<Operation>) -> Result<bool>;

        /// Get `TaskData` values for all tasks in the replica.
        ///
        /// This contains `OptionTaskData` to allow C++ to `take` values out of the vector and use
        /// them as `rust::Box<TaskData>`. Cxx does not support `Vec<Box<_>>`. Cxx also does not
        /// handle `HashMap`, so the result is not a map from uuid to task. The returned Vec is
        /// fully populated, so it is safe to call `take` on each value in the returned Vec once .
        fn all_task_data(&mut self) -> Result<Vec<OptionTaskData>>;

        /// Similar to all_task_data, but returning only pending tasks (those in the working set).
        fn pending_task_data(&mut self) -> Result<Vec<OptionTaskData>>;

        /// Get the UUIDs of all tasks.
        fn all_task_uuids(&mut self) -> Result<Vec<Uuid>>;

        /// Expire old, deleted tasks.
        fn expire_tasks(&mut self) -> Result<()>;

        /// Get an existing task by its UUID.
        fn get_task_data(&mut self, uuid: Uuid) -> Result<OptionTaskData>;

        /// Get the operations for a task task by its UUID.
        fn get_task_operations(&mut self, uuid: Uuid) -> Result<Vec<Operation>>;

        /// Create a new `Task` with the given UUID. If a task with that
        /// UUID already exists, the existing task is returned.
        fn create_task(&mut self, uuid: Uuid, ops: &mut Vec<Operation>) -> Result<Box<Task>>;

        /// Get an existing `Task` by UUID. Returns None if no task with that UUID exists.
        fn get_task(&mut self, uuid: Uuid) -> Result<OptionTask>;

        /// Get all `Task` values in the replica.
        ///
        /// This contains `OptionTask` to allow C++ to `take` values out of the vector and use
        /// them as `rust::Box<Task>`.
        fn all_tasks(&mut self) -> Result<Vec<OptionTask>>;

        /// Similar to all_tasks, but returning only pending tasks (those in the working set).
        fn pending_tasks(&mut self) -> Result<Vec<OptionTask>>;

        /// Return the operations back to and including the last undo point, or since the last sync if
        /// no undo point is found.
        fn get_undo_operations(&mut self) -> Result<Vec<Operation>>;

        /// Get the number of local, un-sync'd operations, excluding undo operations.
        fn num_local_operations(&mut self) -> Result<usize>;

        /// Get the number of (un-synchronized) undo points in storage.
        fn num_undo_points(&mut self) -> Result<usize>;

        /// Rebuild the working set.
        fn rebuild_working_set(&mut self, renumber: bool) -> Result<()>;

        /// Get the working set for this replica.
        fn working_set(&mut self) -> Result<Box<WorkingSet>>;

        /// Sync with a server crated from `ServerConfig::Local`.
        fn sync_to_local(&mut self, server_dir: String, avoid_snapshots: bool) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Remote`.
        fn sync_to_remote(
            &mut self,
            url: String,
            client_id: Uuid,
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Aws` using `AwsCredentials::Profile`.
        fn sync_to_aws_with_profile(
            &mut self,
            region: String,
            bucket: String,
            profile_name: String,
            endpoint_url: String,
            force_path_style: bool,
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Aws` using `AwsCredentials::AccessKey`.
        fn sync_to_aws_with_access_key(
            &mut self,
            region: String,
            bucket: String,
            access_key_id: String,
            secret_access_key: String,
            endpoint_url: String,
            force_path_style: bool,
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Aws` using `AwsCredentials::Default`.
        fn sync_to_aws_with_default_creds(
            &mut self,
            region: String,
            bucket: String,
            endpoint_url: String,
            force_path_style: bool,
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Gcp`.
        ///
        /// An empty value for `credential_path` is converted to `Option::None`.
        fn sync_to_gcp(
            &mut self,
            bucket: String,
            credential_path: String,
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Git`.
        ///
        /// An empty value for `remote` is converted to `Option::None` (local-only mode).
        /// An empty value for `git_path` uses "git" on `$PATH`.
        #[allow(clippy::too_many_arguments)]
        fn sync_to_git(
            &mut self,
            local_path: String,
            branch: String,
            remote: String,
            local_only: bool,
            encryption_secret: &CxxString,
            git_path: String,
            avoid_snapshots: bool,
        ) -> Result<()>;
    }

    // --- OptionTaskData

    /// Wrapper around `Option<Box<TaskData>>`, required since cxx does not support Option<T>.
    ///
    /// Note that if an OptionTaskData containing a task is dropped without calling `take`,
    /// it will leak the contained task. C++ code should be careful to always take.
    struct OptionTaskData {
        maybe_task: *mut TaskData,
    }

    extern "Rust" {
        /// Check if the value contains a task.
        fn is_some(self: &OptionTaskData) -> bool;
        /// Check if the value does not contain a task.
        fn is_none(self: &OptionTaskData) -> bool;
        /// Get the contained task, or panic if there is no task. The OptionTaskData
        /// will be reset to contain None.
        fn take(self: &mut OptionTaskData) -> Box<TaskData>;
    }

    // --- TaskData

    extern "Rust" {
        type TaskData;

        /// Create a new task with the given Uuid.
        fn create_task(uuid: Uuid, ops: &mut Vec<Operation>) -> Box<TaskData>;

        /// Get the task's Uuid.
        fn get_uuid(&self) -> Uuid;

        /// Get a value on this task. If the property exists, returns true and updates
        /// the output parameter. If not, returns false.
        fn get(&self, property: &CxxString, value_out: Pin<&mut CxxString>) -> bool;

        /// Check if the given property is set.
        fn has(&self, property: &CxxString) -> bool;

        /// Enumerate all properties on this task, in arbitrary order.
        fn properties(&self) -> Vec<String>;

        /// Enumerate all properties and their values on this task, in arbitrary order, as a
        /// vector.
        fn items(&self) -> Vec<PropValuePair>;

        /// Update the given property with the given value.
        fn update(&mut self, property: &CxxString, value: &CxxString, ops: &mut Vec<Operation>);

        /// Like `update`, but removing the property by passing None for the value.
        fn update_remove(&mut self, property: &CxxString, ops: &mut Vec<Operation>);

        /// Delete the task. The name is `delete_task` because `delete` is a C++ keyword.
        fn delete_task(&mut self, ops: &mut Vec<Operation>);

    }

    // --- Status

    /// Mirror of `tc::Status`. Used so that taskmap strings don't have to cross the FFI.
    #[repr(i32)]
    enum Status {
        Pending,
        Completed,
        Deleted,
        Recurring,
        Unknown,
    }

    // --- Annotation

    /// An annotation for a task
    #[derive(Debug, PartialEq, Eq, PartialOrd, Ord, Clone)]
    struct Annotation {
        /// Time the annotation was made
        entry: i64,
        /// Content of the annotation
        description: String,
    }

    // --- OptionTask

    /// Wrapper around `Option<Box<Task>>`. Mirrors `OptionTaskData`.
    ///
    /// Note that if an `OptionTask` containing a task is dropped without calling `take`,
    /// it will leak the contained task. C++ code should be careful to always take.
    struct OptionTask {
        maybe_task: *mut Task,
    }

    extern "Rust" {
        /// Check if the value contains a task.
        fn is_some(self: &OptionTask) -> bool;
        /// Check if the value does not contain a task.
        fn is_none(self: &OptionTask) -> bool;
        /// Get the contained task, or panic if there is no task. The `OptionTask`
        /// will be reset to contain None.
        fn take(self: &mut OptionTask) -> Box<Task>;
    }

    // --- Task

    extern "Rust" {
        type Task;

        // First the getters.
        /// Get the task's Uuid.
        fn get_uuid(self: &Task) -> Uuid;
        /// Get the task's status.
        fn get_status(&self) -> Status;
        /// Get the task's description.
        fn get_description(&self) -> String;
        /// Get the task's priority.
        fn get_priority(&self) -> String;
        /// Get the tasks's entry timestamp
        fn get_entry(&self) -> i64;
        /// Get the tasks's wait timestamp
        fn get_wait(&self) -> i64;
        /// Get the tasks's modifier timestamp
        fn get_modified(&self) -> i64;
        /// Get the tasks's due time
        fn get_due(&self) -> i64;
        /// Get the properties timestamp
        fn get_timestamp(&self, property: &CxxString) -> i64;
        /// True if task is waiting.
        fn is_waiting(&self) -> bool;
        /// True if task is active.
        fn is_active(&self) -> bool;
        /// True if task is blocked by another task.
        fn is_blocked(&self) -> bool;
        /// True if task is blocking another task.
        fn is_blocking(&self) -> bool;
        /// Get a property value.
        fn get_value(&self, property: &CxxString, value_out: Pin<&mut CxxString>) -> bool;
        /// Get the task's tags.
        fn get_tags(&self) -> Vec<String>;
        /// True if the task has the given tag. Fails if the string is unparseable.
        fn has_tag(&self, tag: &CxxString) -> Result<bool>;
        /// Get the tasks dependencies.
        fn get_dependencies(&self) -> Vec<Uuid>;
        /// Get the tasks annotations.
        fn get_annotations(&self) -> Vec<Annotation>;
        /// Get a UDA.
        fn get_user_defined_attribute(
            &self,
            key: &CxxString,
            mut value_out: Pin<&mut CxxString>,
        ) -> bool;
        /// Get all UDAs.
        fn get_user_defined_attributes(&self) -> Vec<PropValuePair>;

        // Then the setters. Each takes `ops: &mut Vec<Operation>` and returns `Result<()>`,
        // routing through TaskChampion's bookkeeping.

        /// Set the task's description.
        fn set_description(
            self: &mut Task,
            description: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;
        /// Set the task's priority.
        fn set_priority(
            self: &mut Task,
            priority: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;

        /// Set the task's entry timestamp. A value of `0` clears the timestamp.
        fn set_entry(self: &mut Task, entry: i64, ops: &mut Vec<Operation>) -> Result<()>;
        /// Set the task's wait timestamp. A value of `0` clears the timestamp.
        fn set_wait(self: &mut Task, wait: i64, ops: &mut Vec<Operation>) -> Result<()>;
        /// Set the task's modified timestamp.
        fn set_modified(self: &mut Task, modified: i64, ops: &mut Vec<Operation>) -> Result<()>;
        /// Set the task's due timestamp. A value of `0` clears the timestamp.
        fn set_due(self: &mut Task, due: i64, ops: &mut Vec<Operation>) -> Result<()>;
        /// Set the given timestamp property. A value of `0` clears the timestamp.
        fn set_timestamp(
            self: &mut Task,
            property: &CxxString,
            value: i64,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;

        /// Mark the task as started.
        fn start(self: &mut Task, ops: &mut Vec<Operation>) -> Result<()>;
        /// Mark the task as stopped.
        fn stop(self: &mut Task, ops: &mut Vec<Operation>) -> Result<()>;
        /// Mark the task as completed.
        fn done(self: &mut Task, ops: &mut Vec<Operation>) -> Result<()>;

        /// Add a tag to the task. The tag is parsed from a string, so can fail.
        fn add_tag(self: &mut Task, tag: &CxxString, ops: &mut Vec<Operation>) -> Result<()>;
        /// Remove a tag from the task. The tag is parsed from a string, so can fail.
        fn remove_tag(self: &mut Task, tag: &CxxString, ops: &mut Vec<Operation>) -> Result<()>;

        /// Add an annotation, identified by its entry timestamp.
        fn add_annotation(
            self: &mut Task,
            entry: i64,
            description: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;
        /// Remove an annotation, identified by its entry timestamp.
        fn remove_annotation(self: &mut Task, entry: i64, ops: &mut Vec<Operation>) -> Result<()>;

        /// Set a user-defined attribute (UDA).
        fn set_user_defined_attribute(
            self: &mut Task,
            key: &CxxString,
            value: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;
        /// Remove a user-defined attribute (UDA).
        fn remove_user_defined_attribute(
            self: &mut Task,
            key: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;

        /// Add a dependency on another task.
        fn add_dependency(self: &mut Task, dep: Uuid, ops: &mut Vec<Operation>) -> Result<()>;
        /// Remove a dependency on another task.
        fn remove_dependency(self: &mut Task, dep: Uuid, ops: &mut Vec<Operation>) -> Result<()>;

        /// Set the given property to the given value via `tc::Task::set_value`.
        /// This routes through TaskChampion's bookkeeping.
        fn set_value(
            self: &mut Task,
            property: &CxxString,
            value: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;

        /// Like `set_value`, but removes the property.
        fn set_value_remove(
            self: &mut Task,
            property: &CxxString,
            ops: &mut Vec<Operation>,
        ) -> Result<()>;

        /// Set the task's status via `tc::Task::set_status`.
        fn set_status(self: &mut Task, status: Status, ops: &mut Vec<Operation>) -> Result<()>;
    }

    // --- PropValuePair

    #[derive(Debug, Eq, PartialEq)]
    struct PropValuePair {
        prop: String,
        value: String,
    }

    // --- WorkingSet

    extern "Rust" {
        type WorkingSet;

        /// Get the "length" of the working set: the total number of uuids in the set.
        fn len(&self) -> usize;

        /// Get the largest index in the working set, or zero if the set is empty.
        fn largest_index(&self) -> usize;

        /// True if the length is zero
        fn is_empty(&self) -> bool;

        /// Get the uuid with the given index, if any exists. Returns the nil UUID if
        /// there is no task at that index.
        fn by_index(&self, index: usize) -> Uuid;

        /// Get the index for the given uuid, or zero if it is not in the working set.
        fn by_uuid(&self, uuid: Uuid) -> usize;

        /// Get the entire working set, as a vector indexed by each task's id. For example, the
        /// UUID for task 5 will be at `all_uuids()[5]`. All elements of the vector not corresponding
        /// to a task contain the nil UUID.
        fn all_uuids(&self) -> Vec<Uuid>;
    }
}

#[derive(Debug)]
struct CppError(tc::Error);

impl From<tc::Error> for CppError {
    fn from(err: tc::Error) -> Self {
        CppError(err)
    }
}

use std::sync::OnceLock;

use crate::ffi::Status;

static RUNTIME: OnceLock<tokio::runtime::Runtime> = OnceLock::new();

fn rt() -> &'static tokio::runtime::Runtime {
    RUNTIME.get_or_init(|| {
        tokio::runtime::Builder::new_current_thread()
            .enable_all()
            .build()
            .unwrap()
    })
}

impl std::fmt::Display for CppError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        if let tc::Error::Other(err) = &self.0 {
            // The default `to_string` representation of `anyhow::Error` only shows the "outermost"
            // context, e.g., "Could not connect to server", and omits the juicy details about what
            // actually went wrong. So, join all of those contexts with `: ` for presentation to the C++
            // layer.
            let entire_msg = err
                .chain()
                .skip(1)
                .fold(err.to_string(), |a, b| format!("{a}: {b}"));
            write!(f, "{entire_msg}")
        } else {
            self.0.fmt(f)
        }
    }
}

// --- Uuid

impl From<ffi::Uuid> for tc::Uuid {
    fn from(value: ffi::Uuid) -> Self {
        tc::Uuid::from_bytes(value.v)
    }
}

impl From<&ffi::Uuid> for tc::Uuid {
    fn from(value: &ffi::Uuid) -> Self {
        tc::Uuid::from_bytes(value.v)
    }
}

impl From<tc::Uuid> for ffi::Uuid {
    fn from(uuid: tc::Uuid) -> ffi::Uuid {
        ffi::Uuid {
            v: *uuid.as_bytes(),
        }
    }
}

impl From<&tc::Uuid> for ffi::Uuid {
    fn from(uuid: &tc::Uuid) -> ffi::Uuid {
        ffi::Uuid {
            v: *uuid.as_bytes(),
        }
    }
}

fn uuid_v4() -> ffi::Uuid {
    tc::Uuid::new_v4().into()
}

fn uuid_from_string(uuid: Pin<&CxxString>) -> ffi::Uuid {
    let Ok(uuid) = tc::Uuid::parse_str(uuid.to_str().expect("invalid utf-8")) else {
        panic!("{uuid} is not a valid UUID");
    };
    uuid.into()
}

impl ffi::Uuid {
    #[allow(clippy::inherent_to_string, clippy::wrong_self_convention)]
    fn to_string(&self) -> String {
        tc::Uuid::from(self).as_hyphenated().to_string()
    }

    fn is_nil(&self) -> bool {
        tc::Uuid::from(self).is_nil()
    }
}

// --- Operation and Operations

#[repr(transparent)] // required for safety
pub struct Operation(tc::Operation);

impl Operation {
    fn is_create(&self) -> bool {
        matches!(&self.0, tc::Operation::Create { .. })
    }

    fn is_update(&self) -> bool {
        matches!(&self.0, tc::Operation::Update { .. })
    }

    fn is_delete(&self) -> bool {
        matches!(&self.0, tc::Operation::Delete { .. })
    }

    fn is_undo_point(&self) -> bool {
        matches!(&self.0, tc::Operation::UndoPoint)
    }

    fn get_uuid(&self) -> ffi::Uuid {
        match self.0 {
            tc::Operation::Create { uuid, .. } => uuid,
            tc::Operation::Update { uuid, .. } => uuid,
            tc::Operation::Delete { uuid, .. } => uuid,
            _ => panic!("operation has no uuid"),
        }
        .into()
    }

    fn get_property(&self, mut property_out: Pin<&mut CxxString>) {
        match &self.0 {
            tc::Operation::Update { property, .. } => {
                property_out.as_mut().clear();
                property_out.as_mut().push_str(property);
            }
            _ => panic!("operation is not an update"),
        }
    }

    fn get_value(&self, mut value_out: Pin<&mut CxxString>) -> bool {
        match &self.0 {
            tc::Operation::Update { value, .. } => {
                if let Some(value) = value {
                    value_out.as_mut().clear();
                    value_out.as_mut().push_str(value);
                    true
                } else {
                    false
                }
            }
            _ => panic!("operation is not an update"),
        }
    }

    fn get_old_value(&self, mut old_value_out: Pin<&mut CxxString>) -> bool {
        match &self.0 {
            tc::Operation::Update { old_value, .. } => {
                if let Some(old_value) = old_value {
                    old_value_out.as_mut().clear();
                    old_value_out.as_mut().push_str(old_value);
                    true
                } else {
                    false
                }
            }
            _ => panic!("operation is not an update"),
        }
    }

    fn get_timestamp(&self) -> i64 {
        match &self.0 {
            tc::Operation::Update { timestamp, .. } => timestamp.timestamp(),
            _ => panic!("operation is not an update"),
        }
    }

    fn get_old_task(&self) -> Vec<ffi::PropValuePair> {
        match &self.0 {
            tc::Operation::Delete { old_task, .. } => old_task
                .iter()
                .map(|(p, v)| ffi::PropValuePair {
                    prop: p.into(),
                    value: v.into(),
                })
                .collect(),
            _ => panic!("operation is not a delete"),
        }
    }
}

fn new_operations() -> Vec<Operation> {
    Vec::new()
}

fn add_undo_point(ops: &mut Vec<Operation>) {
    ops.push(Operation(tc::Operation::UndoPoint));
}

// --- Replica

struct Replica(tc::Replica<SqliteStorage>);

impl From<tc::Replica<SqliteStorage>> for Replica {
    fn from(inner: tc::Replica<SqliteStorage>) -> Self {
        Replica(inner)
    }
}

fn new_replica_on_disk(
    taskdb_dir: String,
    create_if_missing: bool,
    read_write: bool,
) -> Result<Box<Replica>, CppError> {
    rt().block_on(async {
        use tc::storage::AccessMode::*;
        let access_mode = if read_write { ReadWrite } else { ReadOnly };
        let storage =
            SqliteStorage::new(PathBuf::from(taskdb_dir), access_mode, create_if_missing).await?;
        Ok(Box::new(tc::Replica::new(storage).into()))
    })
}

/// Utility function for Replica methods using Operations.
fn to_tc_operations(ops: Vec<Operation>) -> Vec<tc::Operation> {
    // SAFETY: Operation is a transparent newtype for tc::Operation, so a Vec of one is
    // a Vec of the other.
    unsafe { std::mem::transmute::<Vec<Operation>, Vec<tc::Operation>>(ops) }
}

/// Utility function for Replica methods using Operations.
fn from_tc_operations(ops: Vec<tc::Operation>) -> Vec<Operation> {
    // SAFETY: Operation is a transparent newtype for tc::Operation, so a Vec of one is
    // a Vec of the other.
    unsafe { std::mem::transmute::<Vec<tc::Operation>, Vec<Operation>>(ops) }
}

impl Replica {
    fn commit_operations(&mut self, ops: Vec<Operation>) -> Result<(), CppError> {
        rt().block_on(async { Ok(self.0.commit_operations(to_tc_operations(ops)).await?) })
    }

    fn commit_reversed_operations(&mut self, ops: Vec<Operation>) -> Result<bool, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .commit_reversed_operations(to_tc_operations(ops))
                .await?)
        })
    }

    fn all_task_data(&mut self) -> Result<Vec<ffi::OptionTaskData>, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .all_task_data()
                .await?
                .drain()
                .map(|(_, t)| Some(t).into())
                .collect())
        })
    }

    fn pending_task_data(&mut self) -> Result<Vec<ffi::OptionTaskData>, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .pending_task_data()
                .await?
                .drain(..)
                .map(|t| Some(t).into())
                .collect())
        })
    }

    fn all_task_uuids(&mut self) -> Result<Vec<ffi::Uuid>, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .all_task_uuids()
                .await?
                .into_iter()
                .map(ffi::Uuid::from)
                .collect())
        })
    }
    fn all_tasks(&mut self) -> Result<Vec<ffi::OptionTask>, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .all_tasks()
                .await?
                .drain()
                .map(|(_, t)| Some(t).into())
                .collect())
        })
    }

    fn pending_tasks(&mut self) -> Result<Vec<ffi::OptionTask>, CppError> {
        rt().block_on(async {
            Ok(self
                .0
                .pending_tasks()
                .await?
                .drain(..)
                .map(|t| Some(t).into())
                .collect())
        })
    }

    fn expire_tasks(&mut self) -> Result<(), CppError> {
        rt().block_on(async { Ok(self.0.expire_tasks().await?) })
    }

    fn get_task_data(&mut self, uuid: ffi::Uuid) -> Result<ffi::OptionTaskData, CppError> {
        rt().block_on(async { Ok(self.0.get_task_data(uuid.into()).await?.into()) })
    }

    fn create_task(
        &mut self,
        uuid: ffi::Uuid,
        ops: &mut Vec<Operation>,
    ) -> Result<Box<Task>, CppError> {
        rt().block_on(async {
            let t = self.0.create_task(uuid.into(), operations_ref(ops)).await?;
            Ok(Box::new(Task(t)))
        })
    }

    fn get_task(&mut self, uuid: ffi::Uuid) -> Result<ffi::OptionTask, CppError> {
        rt().block_on(async { Ok(self.0.get_task(uuid.into()).await?.into()) })
    }
    fn get_task_operations(&mut self, uuid: ffi::Uuid) -> Result<Vec<Operation>, CppError> {
        rt().block_on(async {
            Ok(from_tc_operations(
                self.0.get_task_operations(uuid.into()).await?,
            ))
        })
    }

    fn get_undo_operations(&mut self) -> Result<Vec<Operation>, CppError> {
        rt().block_on(async { Ok(from_tc_operations(self.0.get_undo_operations().await?)) })
    }

    fn num_local_operations(&mut self) -> Result<usize, CppError> {
        rt().block_on(async { Ok(self.0.num_local_operations().await?) })
    }

    fn num_undo_points(&mut self) -> Result<usize, CppError> {
        rt().block_on(async { Ok(self.0.num_undo_points().await?) })
    }

    fn rebuild_working_set(&mut self, renumber: bool) -> Result<(), CppError> {
        rt().block_on(async { Ok(self.0.rebuild_working_set(renumber).await?) })
    }

    fn working_set(&mut self) -> Result<Box<WorkingSet>, CppError> {
        rt().block_on(async { Ok(Box::new(self.0.working_set().await?.into())) })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_local(&mut self, server_dir: String, avoid_snapshots: bool) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Local {
                server_dir: server_dir.into(),
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_remote(
        &mut self,
        url: String,
        client_id: ffi::Uuid,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Remote {
                url,
                client_id: client_id.into(),
                encryption_secret: encryption_secret.as_bytes().to_vec(),
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_aws_with_profile(
        &mut self,
        region: String,
        bucket: String,
        profile_name: String,
        endpoint_url: String,
        force_path_style: bool,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Aws {
                region: Some(region),
                bucket,
                credentials: tc::server::AwsCredentials::Profile { profile_name },
                encryption_secret: encryption_secret.as_bytes().to_vec(),
                endpoint_url: (!endpoint_url.is_empty()).then_some(endpoint_url),
                force_path_style,
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_aws_with_access_key(
        &mut self,
        region: String,
        bucket: String,
        access_key_id: String,
        secret_access_key: String,
        endpoint_url: String,
        force_path_style: bool,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Aws {
                region: Some(region),
                bucket,
                credentials: tc::server::AwsCredentials::AccessKey {
                    access_key_id,
                    secret_access_key,
                },
                encryption_secret: encryption_secret.as_bytes().to_vec(),
                endpoint_url: (!endpoint_url.is_empty()).then_some(endpoint_url),
                force_path_style,
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_aws_with_default_creds(
        &mut self,
        region: String,
        bucket: String,
        endpoint_url: String,
        force_path_style: bool,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Aws {
                region: Some(region),
                bucket,
                credentials: tc::server::AwsCredentials::Default,
                encryption_secret: encryption_secret.as_bytes().to_vec(),
                endpoint_url: (!endpoint_url.is_empty()).then_some(endpoint_url),
                force_path_style,
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_gcp(
        &mut self,
        bucket: String,
        credential_path: String,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Gcp {
                bucket,
                credential_path: if credential_path.is_empty() {
                    None
                } else {
                    Some(credential_path)
                },
                encryption_secret: encryption_secret.as_bytes().to_vec(),
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }

    #[allow(clippy::too_many_arguments)]
    fn sync_to_git(
        &mut self,
        local_path: String,
        branch: String,
        remote: String,
        local_only: bool,
        encryption_secret: &CxxString,
        git_path: String,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        rt().block_on(async {
            let mut server = tc::server::ServerConfig::Git {
                local_path: local_path.into(),
                branch,
                remote: if remote.is_empty() {
                    None
                } else {
                    Some(remote)
                },
                local_only,
                encryption_secret: encryption_secret.as_bytes().to_vec(),
                git_path: if git_path.is_empty() {
                    None
                } else {
                    Some(git_path.into())
                },
            }
            .into_server()
            .await?;
            Ok(self.0.sync(&mut server, avoid_snapshots).await?)
        })
    }
}

// --- OptionTaskData

impl From<Option<tc::TaskData>> for ffi::OptionTaskData {
    fn from(value: Option<tc::TaskData>) -> Self {
        let Some(task) = value else {
            return ffi::OptionTaskData {
                maybe_task: std::ptr::null_mut(),
            };
        };
        let boxed = Box::new(task.into());
        ffi::OptionTaskData {
            maybe_task: Box::into_raw(boxed),
        }
    }
}

impl ffi::OptionTaskData {
    fn is_some(&self) -> bool {
        !self.maybe_task.is_null()
    }

    fn is_none(&self) -> bool {
        self.maybe_task.is_null()
    }

    fn take(&mut self) -> Box<TaskData> {
        let mut ptr = std::ptr::null_mut();
        std::mem::swap(&mut ptr, &mut self.maybe_task);
        if ptr.is_null() {
            panic!("Cannot take an empty OptionTaskdata");
        }
        // SAFETY: this value is not NULL and was created from `Box::into_raw` in the
        // `From<Option<TaskData>>` implementation above.
        unsafe { Box::from_raw(ptr) }
    }
}

// --- TaskData

pub struct TaskData(tc::TaskData);

impl From<tc::TaskData> for TaskData {
    fn from(task: tc::TaskData) -> Self {
        TaskData(task)
    }
}

/// Utility function for TaskData methods.
fn operations_ref(ops: &mut Vec<Operation>) -> &mut Vec<tc::Operation> {
    // SAFETY: Operation is a transparent newtype for tc::Operation, so a Vec of one is a
    // Vec of the other.
    unsafe { std::mem::transmute::<&mut Vec<Operation>, &mut Vec<tc::Operation>>(ops) }
}

/// Convert an `i64` timestamp from C++ into an `Option<Timestamp>`, where `0`
/// means "unset".
fn optional_timestamp(secs: i64) -> Option<tc::chrono::DateTime<tc::chrono::Utc>> {
    if secs == 0 {
        None
    } else {
        Some(tc::utc_timestamp(secs))
    }
}

/// Convert an `Option<Timestamp>` into an `i64` for C++, where `0` means "unset".
/// This is the inverse of `optional_timestamp`.
fn timestamp_secs(ts: Option<tc::chrono::DateTime<tc::chrono::Utc>>) -> i64 {
    ts.map_or(0, |t| t.timestamp())
}

/// Parse a `tc::Tag` from a C++ string.
fn parse_tag(tag: &CxxString) -> Result<tc::Tag, CppError> {
    tag.to_string_lossy()
        .parse::<tc::Tag>()
        .map_err(|e| CppError(tc::Error::Other(e)))
}

fn create_task(uuid: ffi::Uuid, ops: &mut Vec<Operation>) -> Box<TaskData> {
    let t = tc::TaskData::create(uuid.into(), operations_ref(ops));
    Box::new(TaskData(t))
}

impl TaskData {
    fn get_uuid(&self) -> ffi::Uuid {
        self.0.get_uuid().into()
    }

    fn get(&self, property: &CxxString, mut value_out: Pin<&mut CxxString>) -> bool {
        let Some(value) = self.0.get(property.to_string_lossy()) else {
            return false;
        };
        value_out.as_mut().clear();
        value_out.as_mut().push_str(value);
        true
    }

    fn has(&self, property: &CxxString) -> bool {
        self.0.has(property.to_string_lossy())
    }

    fn properties(&self) -> Vec<String> {
        self.0.properties().map(|s| s.to_owned()).collect()
    }

    fn items(&self) -> Vec<ffi::PropValuePair> {
        self.0
            .iter()
            .map(|(p, v)| ffi::PropValuePair {
                prop: p.into(),
                value: v.into(),
            })
            .collect()
    }

    fn update(&mut self, property: &CxxString, value: &CxxString, ops: &mut Vec<Operation>) {
        self.0.update(
            property.to_string_lossy(),
            Some(value.to_string_lossy().into()),
            operations_ref(ops),
        )
    }

    fn update_remove(&mut self, property: &CxxString, ops: &mut Vec<Operation>) {
        self.0
            .update(property.to_string_lossy(), None, operations_ref(ops))
    }

    fn delete_task(&mut self, ops: &mut Vec<Operation>) {
        self.0.delete(operations_ref(ops))
    }
}
// --- OptionTask

impl From<Option<tc::Task>> for ffi::OptionTask {
    fn from(value: Option<tc::Task>) -> Self {
        let Some(t) = value else {
            return ffi::OptionTask {
                maybe_task: std::ptr::null_mut(),
            };
        };
        ffi::OptionTask {
            maybe_task: Box::into_raw(Box::new(Task(t))),
        }
    }
}

impl ffi::OptionTask {
    fn is_some(&self) -> bool {
        !self.maybe_task.is_null()
    }

    fn is_none(&self) -> bool {
        self.maybe_task.is_null()
    }

    fn take(&mut self) -> Box<Task> {
        let ptr = std::mem::replace(&mut self.maybe_task, std::ptr::null_mut());
        if ptr.is_null() {
            panic!("Cannot take an empty OptionTask");
        }
        // SAFETY: this value is not NULL and was created from `Box::into_raw` in the
        // `From<Option<tc::Task>>` implementation above.
        unsafe { Box::from_raw(ptr) }
    }
}

// --- Task

pub struct Task(tc::Task);

impl Task {
    fn get_uuid(&self) -> ffi::Uuid {
        self.0.get_uuid().into()
    }

    fn get_status(&self) -> Status {
        match self.0.get_status() {
            taskchampion::Status::Pending => ffi::Status::Pending,
            taskchampion::Status::Completed => ffi::Status::Completed,
            taskchampion::Status::Deleted => ffi::Status::Deleted,
            taskchampion::Status::Recurring => ffi::Status::Recurring,
            taskchampion::Status::Unknown(_) => ffi::Status::Unknown,
        }
    }

    fn get_description(&self) -> String {
        self.0.get_description().into()
    }

    fn get_priority(&self) -> String {
        self.0.get_priority().into()
    }
    fn get_entry(&self) -> i64 {
        timestamp_secs(self.0.get_entry())
    }
    fn get_wait(&self) -> i64 {
        timestamp_secs(self.0.get_wait())
    }
    fn get_modified(&self) -> i64 {
        timestamp_secs(self.0.get_modified())
    }
    fn get_due(&self) -> i64 {
        timestamp_secs(self.0.get_due())
    }
    fn get_timestamp(&self, property: &CxxString) -> i64 {
        timestamp_secs(self.0.get_timestamp(property.to_string_lossy().as_ref()))
    }

    fn is_waiting(&self) -> bool {
        self.0.is_waiting()
    }
    fn is_active(&self) -> bool {
        self.0.is_active()
    }
    fn is_blocked(&self) -> bool {
        self.0.is_blocked()
    }
    fn is_blocking(&self) -> bool {
        self.0.is_blocking()
    }
    fn get_value(&self, property: &CxxString, mut value_out: Pin<&mut CxxString>) -> bool {
        let Some(value) = self.0.get_value(property.to_string_lossy()) else {
            return false;
        };
        value_out.as_mut().clear();
        value_out.as_mut().push_str(value);
        true
    }

    fn get_tags(&self) -> Vec<String> {
        self.0.get_tags().map(|t| t.to_string()).collect()
    }
    fn has_tag(&self, tag: &CxxString) -> Result<bool, CppError> {
        let tag = parse_tag(tag)?;
        Ok(self.0.has_tag(&tag))
    }
    fn get_dependencies(&self) -> Vec<ffi::Uuid> {
        self.0.get_dependencies().map(|d| d.into()).collect()
    }
    fn get_annotations(&self) -> Vec<ffi::Annotation> {
        self.0
            .get_annotations()
            .map(|a| ffi::Annotation {
                entry: a.entry.timestamp(),
                description: a.description,
            })
            .collect()
    }
    fn get_user_defined_attribute(
        &self,
        key: &CxxString,
        mut value_out: Pin<&mut CxxString>,
    ) -> bool {
        let Some(value) = self
            .0
            .get_user_defined_attribute(key.to_string_lossy().as_ref())
        else {
            return false;
        };
        value_out.as_mut().clear();
        value_out.as_mut().push_str(value);
        true
    }
    fn get_user_defined_attributes(&self) -> Vec<ffi::PropValuePair> {
        self.0
            .get_user_defined_attributes()
            .map(|a| ffi::PropValuePair {
                prop: a.0.to_string(),
                value: a.1.to_string(),
            })
            .collect()
    }

    fn set_value(
        &mut self,
        property: &CxxString,
        value: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.set_value(
            property.to_string_lossy().into_owned(),
            Some(value.to_string_lossy().into_owned()),
            operations_ref(ops),
        )?)
    }

    fn set_value_remove(
        &mut self,
        property: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.set_value(
            property.to_string_lossy().into_owned(),
            None,
            operations_ref(ops),
        )?)
    }

    fn set_status(
        &mut self,
        status: ffi::Status,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        let status = match status {
            ffi::Status::Pending => tc::Status::Pending,
            ffi::Status::Completed => tc::Status::Completed,
            ffi::Status::Deleted => tc::Status::Deleted,
            ffi::Status::Recurring => tc::Status::Recurring,
            // `Status::Unknown` carries no value on this side of the FFI, so it cannot be
            // round-tripped back into `tc::Status::Unknown(value)`.
            _ => {
                return Err(CppError(tc::Error::Usage(format!(
                    "Cannot set task status to {}",
                    status.repr
                ))))
            }
        };
        Ok(self.0.set_status(status, operations_ref(ops))?)
    }

    fn set_description(
        &mut self,
        description: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.set_description(
            description.to_string_lossy().into_owned(),
            operations_ref(ops),
        )?)
    }

    fn set_priority(
        &mut self,
        priority: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self
            .0
            .set_priority(priority.to_string_lossy().into_owned(), operations_ref(ops))?)
    }

    fn set_entry(&mut self, entry: i64, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self
            .0
            .set_entry(optional_timestamp(entry), operations_ref(ops))?)
    }

    fn set_wait(&mut self, wait: i64, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self
            .0
            .set_wait(optional_timestamp(wait), operations_ref(ops))?)
    }

    fn set_modified(&mut self, modified: i64, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self
            .0
            .set_modified(tc::utc_timestamp(modified), operations_ref(ops))?)
    }

    fn set_due(&mut self, due: i64, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self
            .0
            .set_due(optional_timestamp(due), operations_ref(ops))?)
    }

    fn set_timestamp(
        &mut self,
        property: &CxxString,
        value: i64,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.set_timestamp(
            property.to_string_lossy().as_ref(),
            optional_timestamp(value),
            operations_ref(ops),
        )?)
    }

    fn start(&mut self, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self.0.start(operations_ref(ops))?)
    }

    fn stop(&mut self, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self.0.stop(operations_ref(ops))?)
    }

    fn done(&mut self, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self.0.done(operations_ref(ops))?)
    }

    fn add_tag(&mut self, tag: &CxxString, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        let tag = parse_tag(tag)?;
        Ok(self.0.add_tag(&tag, operations_ref(ops))?)
    }

    fn remove_tag(&mut self, tag: &CxxString, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        let tag = parse_tag(tag)?;
        Ok(self.0.remove_tag(&tag, operations_ref(ops))?)
    }

    fn add_annotation(
        &mut self,
        entry: i64,
        description: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        let annotation = tc::Annotation {
            entry: tc::utc_timestamp(entry),
            description: description.to_string_lossy().into_owned(),
        };
        Ok(self.0.add_annotation(annotation, operations_ref(ops))?)
    }

    fn remove_annotation(&mut self, entry: i64, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self
            .0
            .remove_annotation(tc::utc_timestamp(entry), operations_ref(ops))?)
    }

    fn set_user_defined_attribute(
        &mut self,
        key: &CxxString,
        value: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.set_user_defined_attribute(
            key.to_string_lossy().into_owned(),
            value.to_string_lossy().into_owned(),
            operations_ref(ops),
        )?)
    }

    fn remove_user_defined_attribute(
        &mut self,
        key: &CxxString,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.remove_user_defined_attribute(
            key.to_string_lossy().into_owned(),
            operations_ref(ops),
        )?)
    }

    fn add_dependency(&mut self, dep: ffi::Uuid, ops: &mut Vec<Operation>) -> Result<(), CppError> {
        Ok(self.0.add_dependency(dep.into(), operations_ref(ops))?)
    }

    fn remove_dependency(
        &mut self,
        dep: ffi::Uuid,
        ops: &mut Vec<Operation>,
    ) -> Result<(), CppError> {
        Ok(self.0.remove_dependency(dep.into(), operations_ref(ops))?)
    }
}
// --- WorkingSet

struct WorkingSet(tc::WorkingSet);

impl From<tc::WorkingSet> for WorkingSet {
    fn from(task: tc::WorkingSet) -> Self {
        WorkingSet(task)
    }
}

impl WorkingSet {
    fn len(&self) -> usize {
        self.0.len()
    }

    fn largest_index(&self) -> usize {
        self.0.largest_index()
    }

    fn is_empty(&self) -> bool {
        self.0.is_empty()
    }

    fn by_index(&self, index: usize) -> ffi::Uuid {
        self.0.by_index(index).unwrap_or_else(tc::Uuid::nil).into()
    }

    fn by_uuid(&self, uuid: ffi::Uuid) -> usize {
        self.0.by_uuid(uuid.into()).unwrap_or(0)
    }

    fn all_uuids(&self) -> Vec<ffi::Uuid> {
        let mut res = vec![tc::Uuid::nil().into(); self.0.largest_index() + 1];
        for (i, uuid) in self.0.iter() {
            res[i] = uuid.into();
        }
        res
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn uuids() {
        let uuid = uuid_v4();
        assert_eq!(uuid.to_string().len(), 36);
    }

    #[test]
    fn operations() {
        cxx::let_cxx_string!(prop = "prop");
        cxx::let_cxx_string!(prop2 = "prop2");
        cxx::let_cxx_string!(value = "value");
        cxx::let_cxx_string!(value2 = "value2");

        let mut operations = new_operations();
        add_undo_point(&mut operations);
        let mut i = 0;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(!operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(operations[i].is_undo_point());

        let uuid = uuid_v4();
        let mut t = create_task(uuid, &mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(operations[i].is_create());
        assert!(!operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);

        t.update(&prop, &value, &mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);
        // Note that `get_value` and `get_old_value` cannot be tested from Rust, as it is not
        // possible to pass a reference to a CxxString and retain ownership of it.
        assert!(operations[i].get_timestamp() > 0);

        t.update(&prop2, &value, &mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);
        assert!(operations[i].get_timestamp() > 0);

        t.update(&prop2, &value2, &mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);
        assert!(operations[i].get_timestamp() > 0);

        t.update_remove(&prop, &mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(operations[i].is_update());
        assert!(!operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);
        assert!(operations[i].get_timestamp() > 0);

        t.delete_task(&mut operations);
        i += 1;
        assert_eq!(operations.len(), i + 1);
        assert!(!operations[i].is_create());
        assert!(!operations[i].is_update());
        assert!(operations[i].is_delete());
        assert!(!operations[i].is_undo_point());
        assert_eq!(operations[i].get_uuid(), uuid);
        assert_eq!(
            operations[i].get_old_task(),
            vec![ffi::PropValuePair {
                prop: "prop2".into(),
                value: "value2".into(),
            },]
        );
    }

    #[test]
    fn operation_counts() {
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();
        let mut operations = new_operations();
        add_undo_point(&mut operations);
        create_task(uuid_v4(), &mut operations);
        create_task(uuid_v4(), &mut operations);
        create_task(uuid_v4(), &mut operations);
        add_undo_point(&mut operations);
        rep.commit_operations(operations).unwrap();
        // Three non-undo-point operations.
        assert_eq!(rep.num_local_operations().unwrap(), 3);
        // Two undo points
        assert_eq!(rep.num_undo_points().unwrap(), 2);
    }

    #[test]
    fn undo_operations() {
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();
        let mut operations = new_operations();
        let (uuid1, uuid2, uuid3) = (uuid_v4(), uuid_v4(), uuid_v4());
        add_undo_point(&mut operations);
        create_task(uuid1, &mut operations);
        add_undo_point(&mut operations);
        create_task(uuid2, &mut operations);
        create_task(uuid3, &mut operations);
        rep.commit_operations(operations).unwrap();

        let undo_ops = rep.get_undo_operations().unwrap();
        assert_eq!(undo_ops.len(), 3);
        assert!(undo_ops[0].is_undo_point());
        assert!(undo_ops[1].is_create());
        assert_eq!(undo_ops[1].get_uuid(), uuid2);
        assert!(undo_ops[2].is_create());
        assert_eq!(undo_ops[2].get_uuid(), uuid3);
    }

    #[test]
    fn task_lists() {
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();
        let mut operations = new_operations();
        add_undo_point(&mut operations);
        create_task(uuid_v4(), &mut operations);
        create_task(uuid_v4(), &mut operations);
        let mut t = create_task(uuid_v4(), &mut operations);
        cxx::let_cxx_string!(status = "status");
        cxx::let_cxx_string!(pending = "pending");
        t.update(&status, &pending, &mut operations);
        rep.commit_operations(operations).unwrap();

        assert_eq!(rep.all_task_data().unwrap().len(), 3);
        assert_eq!(rep.pending_task_data().unwrap().len(), 1);
        assert_eq!(rep.all_task_uuids().unwrap().len(), 3);
    }

    #[test]
    fn expire_tasks() {
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();
        let mut operations = new_operations();
        add_undo_point(&mut operations);
        create_task(uuid_v4(), &mut operations);
        create_task(uuid_v4(), &mut operations);
        create_task(uuid_v4(), &mut operations);
        rep.commit_operations(operations).unwrap();
        rep.expire_tasks().unwrap();
    }

    #[test]
    fn get_task_data() {
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();

        let uuid = uuid_v4();
        assert!(rep.get_task_data(uuid).unwrap().is_none());

        let mut operations = new_operations();
        create_task(uuid, &mut operations);
        rep.commit_operations(operations).unwrap();

        let mut t = rep.get_task_data(uuid).unwrap();
        assert!(t.is_some());
        assert_eq!(t.take().get_uuid(), uuid);
    }

    #[test]
    fn get_task_operations() {
        cxx::let_cxx_string!(prop = "prop");
        cxx::let_cxx_string!(value = "value");
        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();

        let uuid = uuid_v4();
        assert!(rep.get_task_operations(uuid).unwrap().is_empty());

        let mut operations = new_operations();
        let mut t = create_task(uuid, &mut operations);
        t.update(&prop, &value, &mut operations);
        rep.commit_operations(operations).unwrap();

        let ops = rep.get_task_operations(uuid).unwrap();
        assert_eq!(ops.len(), 2);
        assert!(ops[0].is_create());
        assert!(ops[1].is_update());
    }

    #[test]
    fn task_properties() {
        cxx::let_cxx_string!(prop = "prop");
        cxx::let_cxx_string!(prop2 = "prop2");
        cxx::let_cxx_string!(value = "value");

        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();

        let uuid = uuid_v4();
        let mut operations = new_operations();
        let mut t = create_task(uuid, &mut operations);
        t.update(&prop, &value, &mut operations);
        rep.commit_operations(operations).unwrap();

        let t = rep.get_task_data(uuid).unwrap().take();
        assert!(t.has(&prop));
        assert!(!t.has(&prop2));
        // Note that `get` cannot be tested from Rust, as it is not possible to pass a reference to
        // a CxxString and retain ownership of it.

        assert_eq!(t.properties(), vec!["prop".to_string()]);
        assert_eq!(
            t.items(),
            vec![ffi::PropValuePair {
                prop: "prop".into(),
                value: "value".into(),
            }]
        );
    }

    #[test]
    fn working_set() {
        cxx::let_cxx_string!(status = "status");
        cxx::let_cxx_string!(pending = "pending");
        cxx::let_cxx_string!(completed = "completed");
        let (uuid1, uuid2, uuid3) = (uuid_v4(), uuid_v4(), uuid_v4());

        let tmp_dir = tempfile::TempDir::new().unwrap();
        let path = tmp_dir.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();

        let mut operations = new_operations();
        let mut t = create_task(uuid1, &mut operations);
        t.update(&status, &pending, &mut operations);
        rep.commit_operations(operations).unwrap();

        let mut operations = new_operations();
        let mut t = create_task(uuid2, &mut operations);
        t.update(&status, &pending, &mut operations);
        rep.commit_operations(operations).unwrap();

        let mut operations = new_operations();
        let mut t = create_task(uuid3, &mut operations);
        t.update(&status, &completed, &mut operations);
        rep.commit_operations(operations).unwrap();

        rep.rebuild_working_set(false).unwrap();

        let ws = rep.working_set().unwrap();
        assert!(!ws.is_empty());
        assert_eq!(ws.len(), 2);
        assert_eq!(ws.largest_index(), 2);
        assert_eq!(ws.by_index(1), uuid1);
        assert_eq!(ws.by_uuid(uuid2), 2);
        assert_eq!(ws.by_index(100), tc::Uuid::nil().into());
        assert_eq!(ws.by_uuid(uuid3), 0);
        assert_eq!(ws.all_uuids(), vec![tc::Uuid::nil().into(), uuid1, uuid2]);
    }

    /// Create a replica with a single minimal task, and return both. The task
    /// is re-read from the replica.
    fn replica_with_task(tmp: &tempfile::TempDir) -> (Box<Replica>, Box<Task>) {
        let path = tmp.path().to_str().unwrap().to_string();
        let mut rep = new_replica_on_disk(path, true, true).unwrap();
        let uuid = uuid_v4();
        let mut ops = new_operations();
        rep.create_task(uuid, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(uuid).unwrap().take();
        (rep, task)
    }

    #[test]
    fn task_set_string_fields() {
        cxx::let_cxx_string!(description = "a description");
        cxx::let_cxx_string!(priority = "H");
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        let mut ops = new_operations();
        task.set_description(&description, &mut ops).unwrap();
        task.set_priority(&priority, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();

        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert_eq!(task.get_description(), "a description");
        assert_eq!(task.get_priority(), "H");
    }

    #[test]
    fn task_set_timestamps() {
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        cxx::let_cxx_string!(scheduled = "scheduled");
        let mut ops = new_operations();
        task.set_entry(1000, &mut ops).unwrap();
        task.set_wait(2000, &mut ops).unwrap();
        task.set_modified(3000, &mut ops).unwrap();
        task.set_due(4000, &mut ops).unwrap();
        task.set_timestamp(&scheduled, 5000, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();

        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        assert_eq!(task.get_entry(), 1000);
        assert_eq!(task.get_wait(), 2000);
        assert_eq!(task.get_modified(), 3000);
        assert_eq!(task.get_due(), 4000);
        assert_eq!(task.get_timestamp(&scheduled), 5000);

        // A value of 0 clears the timestamp, which reads back as 0 ("unset").
        let mut ops = new_operations();
        task.set_due(0, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert_eq!(task.get_due(), 0);
    }

    #[test]
    fn task_start_stop_done() {
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        let mut ops = new_operations();
        task.start(&mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(task.is_active());

        let mut ops = new_operations();
        task.stop(&mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(!task.is_active());

        let mut ops = new_operations();
        task.done(&mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(matches!(task.get_status(), ffi::Status::Completed));
    }

    #[test]
    fn task_set_status_unknown_is_error() {
        let tmp = tempfile::TempDir::new().unwrap();
        let (_rep, mut task) = replica_with_task(&tmp);

        // `Status::Unknown` cannot be round-tripped, so it is an error rather than a panic.
        let mut ops = new_operations();
        assert!(task.set_status(ffi::Status::Unknown, &mut ops).is_err());

        // The same goes for a value C++ made up that matches no variant.
        let mut ops = new_operations();
        assert!(task.set_status(ffi::Status { repr: 99 }, &mut ops).is_err());
    }

    #[test]
    fn task_tags() {
        cxx::let_cxx_string!(tag = "next");
        cxx::let_cxx_string!(bad_tag = "not a valid tag");
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        // Parsing an invalid tag is an error and does not panic.
        let mut ops = new_operations();
        assert!(task.add_tag(&bad_tag, &mut ops).is_err());

        let mut ops = new_operations();
        task.add_tag(&tag, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(task.has_tag(&tag).unwrap());
        // get_tags also returns synthetic tags (e.g. PENDING), so check membership.
        assert!(task.get_tags().contains(&"next".to_string()));

        let mut ops = new_operations();
        task.remove_tag(&tag, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(!task.has_tag(&tag).unwrap());
    }

    #[test]
    fn task_annotations() {
        cxx::let_cxx_string!(description = "an annotation");
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        let mut ops = new_operations();
        task.add_annotation(1000, &description, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        let annotations = task.get_annotations();
        assert_eq!(annotations.len(), 1);
        assert_eq!(annotations[0].entry, 1000);
        assert_eq!(annotations[0].description, "an annotation");

        let mut ops = new_operations();
        task.remove_annotation(1000, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(task.get_annotations().is_empty());
    }

    #[test]
    fn task_user_defined_attributes() {
        cxx::let_cxx_string!(key = "estimate");
        cxx::let_cxx_string!(value = "3h");
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);

        let mut ops = new_operations();
        task.set_user_defined_attribute(&key, &value, &mut ops)
            .unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        let udas = task.get_user_defined_attributes();
        assert_eq!(
            udas,
            vec![ffi::PropValuePair {
                prop: "estimate".into(),
                value: "3h".into(),
            }]
        );

        let mut ops = new_operations();
        task.remove_user_defined_attribute(&key, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(task.get_user_defined_attributes().is_empty());
    }

    #[test]
    fn task_dependencies() {
        let tmp = tempfile::TempDir::new().unwrap();
        let (mut rep, mut task) = replica_with_task(&tmp);
        let dep = uuid_v4();

        let mut ops = new_operations();
        task.add_dependency(dep, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let mut task = rep.get_task(task.get_uuid()).unwrap().take();
        assert_eq!(task.get_dependencies(), vec![dep]);

        let mut ops = new_operations();
        task.remove_dependency(dep, &mut ops).unwrap();
        rep.commit_operations(ops).unwrap();
        let task = rep.get_task(task.get_uuid()).unwrap().take();
        assert!(task.get_dependencies().is_empty());
    }
}

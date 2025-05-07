use cxx::CxxString;
use std::path::PathBuf;
use std::pin::Pin;
use taskchampion as tc;

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

        /// Create a new in-memory replica, such as for testing.
        fn new_replica_in_memory() -> Result<Box<Replica>>;

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

        /// Simiar to all_task_data, but returing only pending tasks (those in the working set).
        fn pending_task_data(&mut self) -> Result<Vec<OptionTaskData>>;

        /// Get the UUIDs of all tasks.
        fn all_task_uuids(&mut self) -> Result<Vec<Uuid>>;

        /// Expire old, deleted tasks.
        fn expire_tasks(&mut self) -> Result<()>;

        /// Get an existing task by its UUID.
        fn get_task_data(&mut self, uuid: Uuid) -> Result<OptionTaskData>;

        /// Get the operations for a task task by its UUID.
        fn get_task_operations(&mut self, uuid: Uuid) -> Result<Vec<Operation>>;

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
            encryption_secret: &CxxString,
            avoid_snapshots: bool,
        ) -> Result<()>;

        /// Sync with a server created from `ServerConfig::Aws` using `AwsCredentials::Default`.
        fn sync_to_aws_with_default_creds(
            &mut self,
            region: String,
            bucket: String,
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
                .fold(err.to_string(), |a, b| format!("{}: {}", a, b));
            write!(f, "{}", entire_msg)
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
        panic!("{} is not a valid UUID", uuid);
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

struct Replica(tc::Replica);

impl From<tc::Replica> for Replica {
    fn from(inner: tc::Replica) -> Self {
        Replica(inner)
    }
}

fn new_replica_on_disk(
    taskdb_dir: String,
    create_if_missing: bool,
    read_write: bool,
) -> Result<Box<Replica>, CppError> {
    use tc::storage::AccessMode::*;
    let access_mode = if read_write { ReadWrite } else { ReadOnly };
    let storage = tc::StorageConfig::OnDisk {
        taskdb_dir: PathBuf::from(taskdb_dir),
        create_if_missing,
        access_mode,
    }
    .into_storage()?;
    Ok(Box::new(tc::Replica::new(storage).into()))
}

fn new_replica_in_memory() -> Result<Box<Replica>, CppError> {
    let storage = tc::StorageConfig::InMemory.into_storage()?;
    Ok(Box::new(tc::Replica::new(storage).into()))
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
        Ok(self.0.commit_operations(to_tc_operations(ops))?)
    }

    fn commit_reversed_operations(&mut self, ops: Vec<Operation>) -> Result<bool, CppError> {
        Ok(self.0.commit_reversed_operations(to_tc_operations(ops))?)
    }

    fn all_task_data(&mut self) -> Result<Vec<ffi::OptionTaskData>, CppError> {
        Ok(self
            .0
            .all_task_data()?
            .drain()
            .map(|(_, t)| Some(t).into())
            .collect())
    }

    fn pending_task_data(&mut self) -> Result<Vec<ffi::OptionTaskData>, CppError> {
        Ok(self
            .0
            .pending_task_data()?
            .drain(..)
            .map(|t| Some(t).into())
            .collect())
    }

    fn all_task_uuids(&mut self) -> Result<Vec<ffi::Uuid>, CppError> {
        Ok(self
            .0
            .all_task_uuids()?
            .into_iter()
            .map(ffi::Uuid::from)
            .collect())
    }

    fn expire_tasks(&mut self) -> Result<(), CppError> {
        Ok(self.0.expire_tasks()?)
    }

    fn get_task_data(&mut self, uuid: ffi::Uuid) -> Result<ffi::OptionTaskData, CppError> {
        Ok(self.0.get_task_data(uuid.into())?.into())
    }

    fn get_task_operations(&mut self, uuid: ffi::Uuid) -> Result<Vec<Operation>, CppError> {
        Ok(from_tc_operations(self.0.get_task_operations(uuid.into())?))
    }

    fn get_undo_operations(&mut self) -> Result<Vec<Operation>, CppError> {
        Ok(from_tc_operations(self.0.get_undo_operations()?))
    }

    fn num_local_operations(&mut self) -> Result<usize, CppError> {
        Ok(self.0.num_local_operations()?)
    }

    fn num_undo_points(&mut self) -> Result<usize, CppError> {
        Ok(self.0.num_undo_points()?)
    }

    fn rebuild_working_set(&mut self, renumber: bool) -> Result<(), CppError> {
        Ok(self.0.rebuild_working_set(renumber)?)
    }

    fn working_set(&mut self) -> Result<Box<WorkingSet>, CppError> {
        Ok(Box::new(self.0.working_set()?.into()))
    }

    fn sync_to_local(&mut self, server_dir: String, avoid_snapshots: bool) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Local {
            server_dir: server_dir.into(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
    }

    fn sync_to_remote(
        &mut self,
        url: String,
        client_id: ffi::Uuid,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Remote {
            url,
            client_id: client_id.into(),
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
    }

    fn sync_to_aws_with_profile(
        &mut self,
        region: String,
        bucket: String,
        profile_name: String,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Aws {
            region,
            bucket,
            credentials: tc::server::AwsCredentials::Profile { profile_name },
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
    }

    fn sync_to_aws_with_access_key(
        &mut self,
        region: String,
        bucket: String,
        access_key_id: String,
        secret_access_key: String,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Aws {
            region,
            bucket,
            credentials: tc::server::AwsCredentials::AccessKey {
                access_key_id,
                secret_access_key,
            },
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
    }

    fn sync_to_aws_with_default_creds(
        &mut self,
        region: String,
        bucket: String,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Aws {
            region,
            bucket,
            credentials: tc::server::AwsCredentials::Default,
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
    }

    fn sync_to_gcp(
        &mut self,
        bucket: String,
        credential_path: String,
        encryption_secret: &CxxString,
        avoid_snapshots: bool,
    ) -> Result<(), CppError> {
        let mut server = tc::server::ServerConfig::Gcp {
            bucket,
            credential_path: if credential_path.is_empty() {
                None
            } else {
                Some(credential_path)
            },
            encryption_secret: encryption_secret.as_bytes().to_vec(),
        }
        .into_server()?;
        Ok(self.0.sync(&mut server, avoid_snapshots)?)
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
        let mut rep = new_replica_in_memory().unwrap();
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
        let mut rep = new_replica_in_memory().unwrap();
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
        let mut rep = new_replica_in_memory().unwrap();
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
        let mut rep = new_replica_in_memory().unwrap();
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
        let mut rep = new_replica_in_memory().unwrap();

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
        let mut rep = new_replica_in_memory().unwrap();

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

        let mut rep = new_replica_in_memory().unwrap();

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

        let mut rep = new_replica_in_memory().unwrap();

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
}

use crate::traits::*;
use crate::types::*;
use crate::util::err_to_ruststring;
use std::ptr::NonNull;
use taskchampion::chrono::{DateTime, Utc};
use taskchampion::storage::{ReplicaOp, TaskMap};
use taskchampion::{Replica, StorageConfig};

#[ffizz_header::item]
#[ffizz(order = 900)]
/// ***** TCReplica *****
///
/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
///
/// # Error Handling
///
/// When a `tc_replica_..` function that returns a TCResult returns TC_RESULT_ERROR, then
/// `tc_replica_error` will return the error message.
///
/// # Safety
///
/// The `*TCReplica` returned from `tc_replica_new…` functions is owned by the caller and
/// must later be freed to avoid a memory leak.
///
/// Any function taking a `*TCReplica` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function;
///  - the memory referenced by the pointer must never be modified by C code; and
///  - except for `tc_replica_free`, ownership of a `*TCReplica` remains with the caller.
///
/// Once passed to `tc_replica_free`, a `*TCReplica` becomes invalid and must not be used again.
///
/// TCReplicas are not threadsafe.
///
/// ```c
/// typedef struct TCReplica TCReplica;
/// ```
pub struct TCReplica {
    /// The wrapped Replica
    inner: Replica,

    /// If true, this replica has an outstanding &mut (for a TaskMut)
    mut_borrowed: bool,

    /// The error from the most recent operation, if any
    error: Option<RustString<'static>>,
}

impl PassByPointer for TCReplica {}

impl TCReplica {
    /// Mutably borrow the inner Replica
    pub(crate) fn borrow_mut(&mut self) -> &mut Replica {
        if self.mut_borrowed {
            panic!("replica is already borrowed");
        }
        self.mut_borrowed = true;
        &mut self.inner
    }

    /// Release the borrow made by [`borrow_mut`]
    pub(crate) fn release_borrow(&mut self) {
        if !self.mut_borrowed {
            panic!("replica is not borrowed");
        }
        self.mut_borrowed = false;
    }
}

impl From<Replica> for TCReplica {
    fn from(rep: Replica) -> TCReplica {
        TCReplica {
            inner: rep,
            mut_borrowed: false,
            error: None,
        }
    }
}

/// Utility function to allow using `?` notation to return an error value.  This makes
/// a mutable borrow, because most Replica methods require a `&mut`.
fn wrap<T, F>(rep: *mut TCReplica, f: F, err_value: T) -> T
where
    F: FnOnce(&mut Replica) -> anyhow::Result<T>,
{
    debug_assert!(!rep.is_null());
    // SAFETY:
    //  - rep is not NULL (promised by caller)
    //  - *rep is a valid TCReplica (promised by caller)
    //  - rep is valid for the duration of this function
    //  - rep is not modified by anything else (not threadsafe)
    let rep: &mut TCReplica = unsafe { TCReplica::from_ptr_arg_ref_mut(rep) };
    if rep.mut_borrowed {
        panic!("replica is borrowed and cannot be used");
    }
    rep.error = None;
    match f(&mut rep.inner) {
        Ok(v) => v,
        Err(e) => {
            rep.error = Some(err_to_ruststring(e));
            err_value
        }
    }
}

/// Utility function to allow using `?` notation to return an error value in the constructor.
fn wrap_constructor<T, F>(f: F, error_out: *mut TCString, err_value: T) -> T
where
    F: FnOnce() -> anyhow::Result<T>,
{
    if !error_out.is_null() {
        // SAFETY:
        //  - error_out is not NULL (just checked)
        //  - properly aligned and valid (promised by caller)
        unsafe { *error_out = TCString::default() };
    }

    match f() {
        Ok(v) => v,
        Err(e) => {
            if !error_out.is_null() {
                // SAFETY:
                //  - error_out is not NULL (just checked)
                //  - properly aligned and valid (promised by caller)
                unsafe {
                    TCString::val_to_arg_out(err_to_ruststring(e), error_out);
                }
            }
            err_value
        }
    }
}

#[ffizz_header::item]
#[ffizz(order = 900)]
/// ***** TCReplicaOpType *****
///
/// ```c
/// enum TCReplicaOpType
/// #ifdef __cplusplus
///   : uint32_t
/// #endif // __cplusplus
/// {
///     Create = 0,
///     Delete = 1,
///     Update = 2,
///     UndoPoint = 3,
/// };
/// #ifndef __cplusplus
/// typedef uint32_t TCReplicaOpType;
/// #endif // __cplusplus
/// ```
#[derive(Debug)]
#[derive(Default)]
#[repr(u32)]
pub enum TCReplicaOpType {
    Create = 0,
    Delete = 1,
    Update = 2,
    UndoPoint = 3,
    #[default]
    Error = 4,
}

impl PassByValue for TCReplicaOpType {
    type RustType = u32;

    unsafe fn from_ctype(self) -> Self::RustType {
        self as u32
    }

    fn as_ctype(arg: u32) -> Self {
        match arg {
            0 => TCReplicaOpType::Create,
            1 => TCReplicaOpType::Delete,
            2 => TCReplicaOpType::Update,
            3 => TCReplicaOpType::UndoPoint,
            _ => panic!("Bad TCReplicaOpType."),
        }
    }

}

#[ffizz_header::item]
#[ffizz(order = 901)]
/// Create a new TCReplica with an in-memory database.  The contents of the database will be
/// lost when it is freed with tc_replica_free.
///
/// ```c
/// EXTERN_C struct TCReplica *tc_replica_new_in_memory(void);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_new_in_memory() -> *mut TCReplica {
    let storage = StorageConfig::InMemory
        .into_storage()
        .expect("in-memory always succeeds");
    // SAFETY:
    // - caller promises to free this value
    unsafe { TCReplica::from(Replica::new(storage)).return_ptr() }
}

#[ffizz_header::item]
#[ffizz(order = 901)]
/// Create a new TCReplica with an on-disk database having the given filename.  On error, a string
/// is written to the error_out parameter (if it is not NULL) and NULL is returned.  The caller
/// must free this string.
///
/// ```c
/// EXTERN_C struct TCReplica *tc_replica_new_on_disk(struct TCString path,
///                                          bool create_if_missing,
///                                          struct TCString *error_out);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_new_on_disk(
    path: TCString,
    create_if_missing: bool,
    error_out: *mut TCString,
) -> *mut TCReplica {
    wrap_constructor(
        || {
            // SAFETY:
            //  - path is valid (promised by caller)
            //  - caller will not use path after this call (convention)
            let mut path = unsafe { TCString::val_from_arg(path) };
            let storage = StorageConfig::OnDisk {
                taskdb_dir: path.to_path_buf_mut()?,
                create_if_missing,
            }
            .into_storage()?;

            // SAFETY:
            // - caller promises to free this value
            Ok(unsafe { TCReplica::from(Replica::new(storage)).return_ptr() })
        },
        error_out,
        std::ptr::null_mut(),
    )
}

impl From<TCKVList> for TaskMap {
    fn from(kvlist: TCKVList) -> TaskMap {
        // TODO SAFETY:
        let vec = unsafe { Vec::from_raw_parts(kvlist.items, kvlist.len, kvlist._capacity) };

        let mut taskmap = TaskMap::new();
        vec.into_iter().for_each(|kv| {
            // SAFETY:
            //  - key is valid (promised by caller)
            //  - caller will not use key after this call (convention)
            let key_ruststring = unsafe { TCString::val_from_arg(kv.key) };
            // SAFETY:
            //  - value is valid (promised by caller)
            //  - caller will not use value after this call (convention)
            let value_ruststring = unsafe { TCString::val_from_arg(kv.value) };

            taskmap.insert(
                key_ruststring.into_string().unwrap(),
                value_ruststring.into_string().unwrap(),
            );
        });
        taskmap
    }
}

#[ffizz_header::item]
#[ffizz(order = 901)]
/// ***** TCReplicaOp *****
///
/// ```c
/// struct TCReplicaOp {
///     TCReplicaOpType operation_type;
///     TCUuid uuid;
///     TCKVList old_task;
///     TCString property;
///     TCString old_value;
///     TCString value;
///     TCString timestamp;
/// };
///
/// typedef struct TCReplicaOp TCReplicaOp;
/// ```
#[derive(Debug)]
#[derive(Default)]
#[repr(C)]
pub struct TCReplicaOp {
    operation_type: TCReplicaOpType,
    uuid: TCUuid,
    old_task: TCKVList,
    property: TCString,
    old_value: TCString,
    value: TCString,
    timestamp: TCString,
}

impl From<ReplicaOp> for TCReplicaOp {
    fn from(replica_op: ReplicaOp) -> TCReplicaOp {
        match replica_op {
            ReplicaOp::Create { uuid } => TCReplicaOp {
                operation_type: TCReplicaOpType::Create,
                // SAFETY:
                //  - caller promises to free this value.
                uuid: unsafe { TCUuid::return_val(uuid) },
                ..Default::default()
            },
            ReplicaOp::Delete { uuid, old_task } => TCReplicaOp {
                operation_type: TCReplicaOpType::Delete,
                // SAFETY:
                //  - caller promises to free this value.
                uuid: unsafe { TCUuid::return_val(uuid) },
                old_task: TCKVList::from(old_task),
                ..Default::default()
            },
            ReplicaOp::Update {
                uuid,
                property,
                old_value,
                value,
                timestamp,
            } => {
                let property_ruststring = RustString::String(property);
                let old_value_ruststring = RustString::String(old_value.unwrap_or_default());
                let value_ruststring = RustString::String(value.unwrap_or_default());
                let timestamp_ruststring = RustString::String(timestamp.to_string());

                TCReplicaOp {
                    operation_type: TCReplicaOpType::Update,
                    // SAFETY:
                    //  - caller promises to free this value.
                    uuid: unsafe { TCUuid::return_val(uuid) },
                    // SAFETY:
                    //  - caller promises to free this value.
                    property: unsafe { TCString::return_val(property_ruststring) },
                    // SAFETY:
                    //  - caller promises to free this value.
                    old_value: unsafe { TCString::return_val(old_value_ruststring) },
                    // SAFETY:
                    //  - caller promises to free this value.
                    value: unsafe { TCString::return_val(value_ruststring) },
                    // SAFETY:
                    //  - caller promises to free this value.
                    timestamp: unsafe { TCString::return_val(timestamp_ruststring) },
                    ..Default::default()
                }
            }
            ReplicaOp::UndoPoint => TCReplicaOp {
                operation_type: TCReplicaOpType::UndoPoint,
                ..Default::default()
            },
        }
    }
}

impl From<TCReplicaOp> for ReplicaOp {
    fn from(tc_replica_op: TCReplicaOp) -> ReplicaOp {
        match tc_replica_op {
            TCReplicaOp {
                operation_type: TCReplicaOpType::Create,
                uuid,
                ..
            } => ReplicaOp::Create {
                // SAFETY:
                //  - uuid is a valid TCUuid (all byte patterns are valid)
                uuid: unsafe { TCUuid::val_from_arg(uuid) },
            },
            TCReplicaOp {
                operation_type: TCReplicaOpType::Delete,
                uuid,
                old_task,
                ..
            } => ReplicaOp::Delete {
                // SAFETY:
                //  - uuid is a valid TCUuid (all byte patterns are valid)
                uuid: unsafe { TCUuid::val_from_arg(uuid) },
                old_task: TaskMap::from(old_task),
            },
            TCReplicaOp {
                operation_type: TCReplicaOpType::Update,
                uuid,
                property,
                old_value,
                value,
                timestamp,
                ..
            } => {
                // SAFETY:
                //  - uuid is a valid TCUuid (all byte patterns are valid)
                let uuid_ruststring = unsafe { TCUuid::val_from_arg(uuid) };
                // SAFETY:
                //  - property is valid (promised by caller)
                //  - caller will not use property after this call (convention)
                let property_ruststring = unsafe { TCString::val_from_arg(property) };
                // SAFETY:
                //  - old_value is valid (promised by caller)
                //  - caller will not use old_value after this call (convention)
                let old_value_ruststring = unsafe { TCString::val_from_arg(old_value) };
                // SAFETY:
                //  - value is valid (promised by caller)
                //  - caller will not use value after this call (convention)
                let value_ruststring = unsafe { TCString::val_from_arg(value) };
                // SAFETY:
                //  - timestamp is valid (promised by caller)
                //  - caller will not use timestamp after this call (convention)
                let timestamp_ruststring = unsafe { TCString::val_from_arg(timestamp) };

                ReplicaOp::Update {
                    uuid: uuid_ruststring,
                    property: property_ruststring.into_string().unwrap(),
                    old_value: Some(old_value_ruststring.into_string().unwrap()),
                    value: Some(value_ruststring.into_string().unwrap()),
                    timestamp: timestamp_ruststring
                        .into_string()
                        .unwrap()
                        .parse::<DateTime<Utc>>()
                        .unwrap(),
                }
            }
            TCReplicaOp {
                operation_type: TCReplicaOpType::UndoPoint,
                ..
            } => ReplicaOp::UndoPoint,
            TCReplicaOp {
                operation_type: TCReplicaOpType::Error,
                ..
            } => panic!("This shouldn't be possible."),
        }
    }
}

#[ffizz_header::item]
#[ffizz(order = 901)]
/// ***** TCReplicaOpList *****
///
/// ```c
/// struct TCReplicaOpList {
///     struct TCReplicaOp *items;
///     size_t len;
///     size_t capacity;
/// };
///
/// typedef struct TCReplicaOpList TCReplicaOpList;
/// ```
#[repr(C)]
pub struct TCReplicaOpList {
    items: *mut TCReplicaOp,
    len: usize,
    capacity: usize,
}

impl CList for TCReplicaOpList {
    type Element = TCReplicaOp;

    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self {
        TCReplicaOpList {
            len,
            capacity: cap,
            items,
        }
    }

    fn slice(&mut self) -> &mut [Self::Element] {
        // SAFETY:
        //  - because we have &mut self, we have read/write access to items[0..len]
        //  - all items are properly initialized Element's
        //  - return value lifetime is equal to &mmut self's, so access is exclusive
        //  - items and len came from Vec, so total size is < isize::MAX
        unsafe { std::slice::from_raw_parts_mut(self.items, self.len) }
    }

    fn into_raw_parts(self) -> (*mut Self::Element, usize, usize) {
        (self.items, self.len, self.capacity)
    }
}

impl From<Vec<ReplicaOp>> for TCReplicaOpList {
    fn from(replica_op_list: Vec<ReplicaOp>) -> TCReplicaOpList {
        // XXX Remove prints
        println!("{:#?}", replica_op_list);
        let tc_replica_op_list: Vec<TCReplicaOp> = replica_op_list.into_iter().map(|op| TCReplicaOp::from(op)).collect();
        println!("{:#?}", tc_replica_op_list);
        TCReplicaOpList {
            items: tc_replica_op_list.as_ptr() as *mut TCReplicaOp,
            len: tc_replica_op_list.len(),
            capacity: tc_replica_op_list.capacity(),
        }
    }
}

impl From<TCReplicaOpList> for Vec<ReplicaOp> {
    fn from(tc_replica_op_list: TCReplicaOpList) -> Vec<ReplicaOp> {
        // TODO SAFETY:
        let tc_replica_op_vec = unsafe {
            Vec::from_raw_parts(
                tc_replica_op_list.items,
                tc_replica_op_list.len,
                tc_replica_op_list.capacity,
            )
        };

        tc_replica_op_vec
            .into_iter()
            .map(|tc_op| ReplicaOp::from(tc_op))
            .collect()
    }
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get a list of all tasks in the replica.
///
/// Returns a TCTaskList with a NULL items field on error.
///
/// ```c
/// EXTERN_C struct TCTaskList tc_replica_all_tasks(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_all_tasks(rep: *mut TCReplica) -> TCTaskList {
    wrap(
        rep,
        |rep| {
            // note that the Replica API returns a hashmap here, but we discard
            // the keys and return a simple list.  The task UUIDs are available
            // from task.get_uuid(), so information is not lost.
            let tasks: Vec<_> = rep
                .all_tasks()?
                .drain()
                .map(|(_uuid, t)| {
                    Some(
                        NonNull::new(
                            // SAFETY:
                            // - caller promises to free this value (via freeing the list)
                            unsafe { TCTask::from(t).return_ptr() },
                        )
                        .expect("TCTask::return_ptr returned NULL"),
                    )
                })
                .collect();
            // SAFETY:
            //  - value is not allocated and need not be freed
            Ok(unsafe { TCTaskList::return_val(tasks) })
        },
        TCTaskList::null_value(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get a list of all uuids for tasks in the replica.
///
/// Returns a TCUuidList with a NULL items field on error.
///
/// The caller must free the UUID list with `tc_uuid_list_free`.
///
/// ```c
/// EXTERN_C struct TCUuidList tc_replica_all_task_uuids(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_all_task_uuids(rep: *mut TCReplica) -> TCUuidList {
    wrap(
        rep,
        |rep| {
            let uuids: Vec<_> = rep
                .all_task_uuids()?
                .drain(..)
                // SAFETY:
                //  - value is not allocated and need not be freed
                .map(|uuid| unsafe { TCUuid::return_val(uuid) })
                .collect();
            // SAFETY:
            //  - value will be freed (promised by caller)
            Ok(unsafe { TCUuidList::return_val(uuids) })
        },
        TCUuidList::null_value(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get the current working set for this replica.  The resulting value must be freed
/// with tc_working_set_free.
///
/// Returns NULL on error.
///
/// ```c
/// EXTERN_C struct TCWorkingSet *tc_replica_working_set(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_working_set(rep: *mut TCReplica) -> *mut TCWorkingSet {
    wrap(
        rep,
        |rep| {
            let ws = rep.working_set()?;
            // SAFETY:
            // - caller promises to free this value
            Ok(unsafe { TCWorkingSet::return_ptr(ws.into()) })
        },
        std::ptr::null_mut(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get an existing task by its UUID.
///
/// Returns NULL when the task does not exist, and on error.  Consult tc_replica_error
/// to distinguish the two conditions.
///
/// ```c
/// EXTERN_C struct TCTask *tc_replica_get_task(struct TCReplica *rep, struct TCUuid tcuuid);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_get_task(rep: *mut TCReplica, tcuuid: TCUuid) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            // SAFETY:
            // - tcuuid is a valid TCUuid (all bytes are valid)
            // - tcuuid is Copy so ownership doesn't matter
            let uuid = unsafe { TCUuid::val_from_arg(tcuuid) };
            if let Some(task) = rep.get_task(uuid)? {
                // SAFETY:
                // - caller promises to free this task
                Ok(unsafe { TCTask::from(task).return_ptr() })
            } else {
                Ok(std::ptr::null_mut())
            }
        },
        std::ptr::null_mut(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
///
/// ```c
/// EXTERN_C struct TCTask *tc_replica_new_task(struct TCReplica *rep,
///                                    enum TCStatus status,
///                                    struct TCString description);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_new_task(
    rep: *mut TCReplica,
    status: TCStatus,
    description: TCString,
) -> *mut TCTask {
    // SAFETY:
    //  - description is valid (promised by caller)
    //  - caller will not use description after this call (convention)
    let mut description = unsafe { TCString::val_from_arg(description) };
    wrap(
        rep,
        |rep| {
            let task = rep.new_task(status.into(), description.as_str()?.to_string())?;
            // SAFETY:
            // - caller promises to free this task
            Ok(unsafe { TCTask::from(task).return_ptr() })
        },
        std::ptr::null_mut(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Create a new task.  The task must not already exist.
///
/// Returns the task, or NULL on error.
///
/// ```c
/// EXTERN_C struct TCTask *tc_replica_import_task_with_uuid(struct TCReplica *rep, struct TCUuid tcuuid);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_import_task_with_uuid(
    rep: *mut TCReplica,
    tcuuid: TCUuid,
) -> *mut TCTask {
    wrap(
        rep,
        |rep| {
            // SAFETY:
            // - tcuuid is a valid TCUuid (all bytes are valid)
            // - tcuuid is Copy so ownership doesn't matter
            let uuid = unsafe { TCUuid::val_from_arg(tcuuid) };
            let task = rep.import_task_with_uuid(uuid)?;
            // SAFETY:
            // - caller promises to free this task
            Ok(unsafe { TCTask::from(task).return_ptr() })
        },
        std::ptr::null_mut(),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Synchronize this replica with a server.
///
/// The `server` argument remains owned by the caller, and must be freed explicitly.
///
/// ```c
/// EXTERN_C TCResult tc_replica_sync(struct TCReplica *rep, struct TCServer *server, bool avoid_snapshots);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_sync(
    rep: *mut TCReplica,
    server: *mut TCServer,
    avoid_snapshots: bool,
) -> TCResult {
    wrap(
        rep,
        |rep| {
            debug_assert!(!server.is_null());
            // SAFETY:
            //  - server is not NULL
            //  - *server is a valid TCServer (promised by caller)
            //  - server is valid for the lifetime of tc_replica_sync (not threadsafe)
            //  - server will not be accessed simultaneously (not threadsafe)
            let server = unsafe { TCServer::from_ptr_arg_ref_mut(server) };
            rep.sync(server.as_mut(), avoid_snapshots)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Return undo local operations until the most recent UndoPoint.
///
/// ```c
/// EXTERN_C TCReplicaOpList tc_replica_get_undo_ops(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_get_undo_ops(rep: *mut TCReplica) -> TCReplicaOpList {
    wrap(
        rep,
        |rep| Ok(TCReplicaOpList::from(rep.get_undo_ops()?)),
        TCReplicaOpList::from(Vec::new()),
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Undo local operations in storage.
///
/// If undone_out is not NULL, then on success it is set to 1 if operations were undone, or 0 if
/// there are no operations that can be done.
///
/// ```c
/// EXTERN_C TCResult tc_replica_commit_undo_ops(struct TCReplica *rep, TCReplicaOpList tc_undo_ops, int32_t *undone_out);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_commit_undo_ops(
    rep: *mut TCReplica,
    tc_undo_ops: TCReplicaOpList,
    undone_out: *mut i32,
) -> TCResult {
    wrap(
        rep,
        |rep| {
            let undo_ops: Vec<ReplicaOp> = Vec::from(tc_undo_ops);
            let undone = i32::from(rep.commit_undo_ops(undo_ops)?);
            if !undone_out.is_null() {
                // SAFETY:
                //  - undone_out is not NULL (just checked)
                //  - undone_out is properly aligned (implicitly promised by caller)
                unsafe { *undone_out = undone };
            }
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get the number of local, un-synchronized operations (not including undo points), or -1 on
/// error.
///
/// ```c
/// EXTERN_C int64_t tc_replica_num_local_operations(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_num_local_operations(rep: *mut TCReplica) -> i64 {
    wrap(
        rep,
        |rep| {
            let count = rep.num_local_operations()? as i64;
            Ok(count)
        },
        -1,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get the number of undo points (number of undo calls possible), or -1 on error.
///
/// ```c
/// EXTERN_C int64_t tc_replica_num_undo_points(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_num_undo_points(rep: *mut TCReplica) -> i64 {
    wrap(
        rep,
        |rep| {
            let count = rep.num_undo_points()? as i64;
            Ok(count)
        },
        -1,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Add an UndoPoint, if one has not already been added by this Replica.  This occurs automatically
/// when a change is made.  The `force` flag allows forcing a new UndoPoint even if one has already
/// been created by this Replica, and may be useful when a Replica instance is held for a long time
/// and used to apply more than one user-visible change.
///
/// ```c
/// EXTERN_C TCResult tc_replica_add_undo_point(struct TCReplica *rep, bool force);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_add_undo_point(rep: *mut TCReplica, force: bool) -> TCResult {
    wrap(
        rep,
        |rep| {
            rep.add_undo_point(force)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Rebuild this replica's working set, based on whether tasks are pending or not.  If `renumber`
/// is true, then existing tasks may be moved to new working-set indices; in any case, on
/// completion all pending tasks are in the working set and all non- pending tasks are not.
///
/// ```c
/// EXTERN_C TCResult tc_replica_rebuild_working_set(struct TCReplica *rep, bool renumber);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_rebuild_working_set(
    rep: *mut TCReplica,
    renumber: bool,
) -> TCResult {
    wrap(
        rep,
        |rep| {
            rep.rebuild_working_set(renumber)?;
            Ok(TCResult::Ok)
        },
        TCResult::Error,
    )
}

#[ffizz_header::item]
#[ffizz(order = 902)]
/// Get the latest error for a replica, or a string with NULL ptr if no error exists.  Subsequent
/// calls to this function will return NULL.  The rep pointer must not be NULL.  The caller must
/// free the returned string.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_error(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_error(rep: *mut TCReplica) -> TCString {
    // SAFETY:
    //  - rep is not NULL (promised by caller)
    //  - *rep is a valid TCReplica (promised by caller)
    //  - rep is valid for the duration of this function
    //  - rep is not modified by anything else (not threadsafe)
    let rep: &mut TCReplica = unsafe { TCReplica::from_ptr_arg_ref_mut(rep) };
    if let Some(rstring) = rep.error.take() {
        // SAFETY:
        // - caller promises to free this string
        unsafe { TCString::return_val(rstring) }
    } else {
        TCString::default()
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Free a replica.  The replica may not be used after this function returns and must not be freed
/// more than once.
///
/// ```c
/// EXTERN_C void tc_replica_free(struct TCReplica *rep);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_free(rep: *mut TCReplica) {
    // SAFETY:
    //  - replica is not NULL (promised by caller)
    //  - replica is valid (promised by caller)
    //  - caller will not use description after this call (promised by caller)
    let replica = unsafe { TCReplica::take_from_ptr_arg(rep) };
    if replica.mut_borrowed {
        panic!("replica is borrowed and cannot be freed");
    }
    drop(replica);
}

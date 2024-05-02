use crate::traits::*;
use crate::types::*;
use crate::util::err_to_ruststring;
use std::ptr::NonNull;
use taskchampion::storage::ReplicaOp;
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
#[derive(Debug, Default)]
#[repr(u32)]
pub enum TCReplicaOpType {
    Create = 0,
    Delete = 1,
    Update = 2,
    UndoPoint = 3,
    #[default]
    Error = 4,
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

#[ffizz_header::item]
#[ffizz(order = 901)]
/// ***** TCReplicaOp *****
///
/// ```c
/// struct TCReplicaOp {
///     TCReplicaOpType operation_type;
///     void* inner;
/// };
///
/// typedef struct TCReplicaOp TCReplicaOp;
/// ```
#[derive(Debug)]
#[repr(C)]
pub struct TCReplicaOp {
    operation_type: TCReplicaOpType,
    inner: Box<ReplicaOp>,
}

impl From<ReplicaOp> for TCReplicaOp {
    fn from(replica_op: ReplicaOp) -> TCReplicaOp {
        match replica_op {
            ReplicaOp::Create { .. } => TCReplicaOp {
                operation_type: TCReplicaOpType::Create,
                inner: Box::new(replica_op),
            },
            ReplicaOp::Delete { .. } => TCReplicaOp {
                operation_type: TCReplicaOpType::Delete,
                inner: Box::new(replica_op),
            },
            ReplicaOp::Update { .. } => TCReplicaOp {
                operation_type: TCReplicaOpType::Update,
                inner: Box::new(replica_op),
            },
            ReplicaOp::UndoPoint => TCReplicaOp {
                operation_type: TCReplicaOpType::UndoPoint,
                inner: Box::new(replica_op),
            },
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
#[derive(Debug)]
pub struct TCReplicaOpList {
    items: *mut TCReplicaOp,
    len: usize,
    capacity: usize,
}

impl Default for TCReplicaOpList {
    fn default() -> Self {
        // SAFETY:
        //  - caller will free this value
        unsafe { TCReplicaOpList::return_val(Vec::new()) }
    }
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
        |rep| {
            // SAFETY:
            //  - caller will free this value, either with tc_replica_commit_undo_ops or
            //  tc_replica_op_list_free.
            Ok(unsafe {
                TCReplicaOpList::return_val(
                    rep.get_undo_ops()?
                        .into_iter()
                        .map(TCReplicaOp::from)
                        .collect(),
                )
            })
        },
        TCReplicaOpList::default(),
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
            // SAFETY:
            // - `tc_undo_ops` is a valid value, as it was acquired from `tc_replica_get_undo_ops`.
            let undo_ops: Vec<ReplicaOp> = unsafe { TCReplicaOpList::val_from_arg(tc_undo_ops) }
                .into_iter()
                .map(|op| *op.inner)
                .collect();
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

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Free a vector of ReplicaOp.  The vector may not be used after this function returns and must not be freed
/// more than once.
///
/// ```c
/// EXTERN_C void tc_replica_op_list_free(struct TCReplicaOpList *oplist);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_list_free(oplist: *mut TCReplicaOpList) {
    debug_assert!(!oplist.is_null());
    // SAFETY:
    // - arg is not NULL (just checked)
    // - `*oplist` is valid (guaranteed by caller not double-freeing this value)
    unsafe {
        TCReplicaOpList::take_val_from_arg(
            oplist,
            // SAFETY:
            //  - value is empty, so the caller need not free it.
            TCReplicaOpList::return_val(Vec::new()),
        )
    };
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return uuid field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_uuid(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_uuid(op: *const TCReplicaOp) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Create { uuid }
    | ReplicaOp::Delete { uuid, .. }
    | ReplicaOp::Update { uuid, .. } = rop
    {
        let uuid_rstr: RustString = uuid.to_string().into();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(uuid_rstr) }
    } else {
        panic!("Operation has no uuid: {:#?}", rop);
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return property field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_property(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_property(op: *const TCReplicaOp) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Update { property, .. } = rop {
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(property.clone().into()) }
    } else {
        panic!("Operation has no property: {:#?}", rop);
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return value field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_value(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_value(op: *const TCReplicaOp) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Update { value, .. } = rop {
        let value_rstr: RustString = value.clone().unwrap_or(String::new()).into();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(value_rstr) }
    } else {
        panic!("Operation has no value: {:#?}", rop);
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return old value field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_old_value(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_old_value(op: *const TCReplicaOp) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Update { old_value, .. } = rop {
        let old_value_rstr: RustString = old_value.clone().unwrap_or(String::new()).into();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(old_value_rstr) }
    } else {
        panic!("Operation has no old value: {:#?}", rop);
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return timestamp field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_timestamp(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_timestamp(op: *const TCReplicaOp) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Update { timestamp, .. } = rop {
        let timestamp_rstr: RustString = timestamp.to_string().into();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(timestamp_rstr) }
    } else {
        panic!("Operation has no timestamp: {:#?}", rop);
    }
}

#[ffizz_header::item]
#[ffizz(order = 903)]
/// Return description field of old task field of ReplicaOp.
///
/// ```c
/// EXTERN_C struct TCString tc_replica_op_get_old_task_description(struct TCReplicaOp *op);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_replica_op_get_old_task_description(
    op: *const TCReplicaOp,
) -> TCString {
    // SAFETY:
    //   - inner is not null
    //   - inner is a living object
    let rop: &ReplicaOp = unsafe { (*op).inner.as_ref() };

    if let ReplicaOp::Delete { old_task, .. } = rop {
        let description_rstr: RustString = old_task["description"].clone().into();
        // SAFETY:
        //  - caller promises to free this string
        unsafe { TCString::return_val(description_rstr) }
    } else {
        panic!("Operation has no timestamp: {:#?}", rop);
    }
}

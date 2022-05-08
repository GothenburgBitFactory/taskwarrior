use crate::traits::*;
use crate::types::*;
use taskchampion::{Uuid, WorkingSet};

/// A TCWorkingSet represents a snapshot of the working set for a replica.  It is not automatically
/// updated based on changes in the replica.  Its lifetime is independent of the replica and it can
/// be freed at any time.
///
/// To iterate over a working set, search indexes 1 through largest_index.
///
/// # Safety
///
/// The `*TCWorkingSet` returned from `tc_replica_working_set` is owned by the caller and
/// must later be freed to avoid a memory leak.  Its lifetime is independent of the replica
/// from which it was generated.
///
/// Any function taking a `*TCWorkingSet` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from `tc_replica_working_set`
///  - the memory referenced by the pointer must never be accessed by C code; and
///  - except for `tc_replica_free`, ownership of a `*TCWorkingSet` remains with the caller.
///
/// Once passed to `tc_replica_free`, a `*TCWorkingSet` becomes invalid and must not be used again.
///
/// TCWorkingSet is not threadsafe.
pub struct TCWorkingSet(WorkingSet);

impl PassByPointer for TCWorkingSet {}

impl From<WorkingSet> for TCWorkingSet {
    fn from(ws: WorkingSet) -> TCWorkingSet {
        TCWorkingSet(ws)
    }
}

/// Utility function to get a shared reference to the underlying WorkingSet.
fn wrap<T, F>(ws: *mut TCWorkingSet, f: F) -> T
where
    F: FnOnce(&WorkingSet) -> T,
{
    // SAFETY:
    //  - ws is not null (promised by caller)
    //  - ws outlives 'a (promised by caller)
    let tcws: &TCWorkingSet = unsafe { TCWorkingSet::from_ptr_arg_ref(ws) };
    f(&tcws.0)
}

/// Get the working set's length, or the number of UUIDs it contains.
#[no_mangle]
pub unsafe extern "C" fn tc_working_set_len(ws: *mut TCWorkingSet) -> usize {
    wrap(ws, |ws| ws.len())
}

/// Get the working set's largest index.
#[no_mangle]
pub unsafe extern "C" fn tc_working_set_largest_index(ws: *mut TCWorkingSet) -> usize {
    wrap(ws, |ws| ws.largest_index())
}

/// Get the UUID for the task at the given index.  Returns true if the UUID exists in the working
/// set.  If not, returns false and does not change uuid_out.
#[no_mangle]
pub unsafe extern "C" fn tc_working_set_by_index(
    ws: *mut TCWorkingSet,
    index: usize,
    uuid_out: *mut TCUuid,
) -> bool {
    debug_assert!(!uuid_out.is_null());
    wrap(ws, |ws| {
        if let Some(uuid) = ws.by_index(index) {
            // SAFETY:
            //  - uuid_out is not NULL (promised by caller)
            //  - alignment is not required
            unsafe { TCUuid::val_to_arg_out(uuid, uuid_out) };
            true
        } else {
            false
        }
    })
}

/// Get the working set index for the task with the given UUID.  Returns 0 if the task is not in
/// the working set.
#[no_mangle]
pub unsafe extern "C" fn tc_working_set_by_uuid(ws: *mut TCWorkingSet, uuid: TCUuid) -> usize {
    wrap(ws, |ws| {
        // SAFETY:
        //  - tcuuid is a valid TCUuid (all byte patterns are valid)
        let uuid: Uuid = unsafe { TCUuid::val_from_arg(uuid) };
        ws.by_uuid(uuid).unwrap_or(0)
    })
}

/// Free a TCWorkingSet.  The given value must not be NULL.  The value must not be used after this
/// function returns, and must not be freed more than once.
#[no_mangle]
pub unsafe extern "C" fn tc_working_set_free(ws: *mut TCWorkingSet) {
    // SAFETY:
    //  - rep is not NULL (promised by caller)
    //  - caller will not use the TCWorkingSet after this (promised by caller)
    let ws = unsafe { TCWorkingSet::take_from_ptr_arg(ws) };
    drop(ws);
}

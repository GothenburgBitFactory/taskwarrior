use libc::c_char;
use std::ffi::{CStr, CString, OsStr};
use std::path::PathBuf;
use taskchampion::Replica as TCReplica;
use taskchampion::StorageConfig;

// TODO: unix-only
use std::os::unix::ffi::OsStrExt;

/// A replica represents an instance of a user's task data, providing an easy interface
/// for querying and modifying that data.
pub struct Replica {
    // TODO: make this an option so that it can be take()n when holding a mut ref
    inner: TCReplica,
    error: Option<CString>,
}

/// Create a new Replica.
///
/// If path is NULL, then an in-memory replica is created.  Otherwise, path is the path to the
/// on-disk storage for this replica.  The path argument is no longer referenced after return.
///
/// Returns NULL on error; see tc_replica_error.
///
/// Replicas are not threadsafe.
#[no_mangle]
pub extern "C" fn tc_replica_new<'a>(path: *const c_char) -> *mut Replica {
    let storage_res = if path.is_null() {
        StorageConfig::InMemory.into_storage()
    } else {
        let path: &'a [u8] = unsafe { CStr::from_ptr(path) }.to_bytes();
        let path: &OsStr = OsStr::from_bytes(path);
        let path: PathBuf = path.to_os_string().into();
        StorageConfig::OnDisk { taskdb_dir: path }.into_storage()
    };

    let storage = match storage_res {
        Ok(storage) => storage,
        // TODO: report errors somehow
        Err(_) => return std::ptr::null_mut(),
    };

    Box::into_raw(Box::new(Replica {
        inner: TCReplica::new(storage),
        error: None,
    }))
}

/// Utility function to safely convert *mut Replica into &mut Replica
fn rep_ref(rep: *mut Replica) -> &'static mut Replica {
    debug_assert!(!rep.is_null());
    unsafe { &mut *rep }
}

fn wrap<'a, T, F>(rep: *mut Replica, f: F, err_value: T) -> T
where
    F: FnOnce(&mut TCReplica) -> anyhow::Result<T>,
{
    let rep: &'a mut Replica = rep_ref(rep);
    match f(&mut rep.inner) {
        Ok(v) => v,
        Err(e) => {
            let error = e.to_string();
            let error = match CString::new(error.as_bytes()) {
                Ok(e) => e,
                Err(_) => CString::new("(invalid error message)".as_bytes()).unwrap(),
            };
            rep.error = Some(error);
            err_value
        }
    }
}

/// Undo local operations until the most recent UndoPoint.
///
/// Returns -1 on error, 0 if there are no local operations to undo, and 1 if operations were
/// undone.
#[no_mangle]
pub extern "C" fn tc_replica_undo<'a>(rep: *mut Replica) -> i32 {
    wrap(rep, |rep| Ok(if rep.undo()? { 1 } else { 0 }), -1)
}

/// Get the latest error for a replica, or NULL if the last operation succeeded.
///
/// The returned string is valid until the next replica operation.
#[no_mangle]
pub extern "C" fn tc_replica_error<'a>(rep: *mut Replica) -> *const c_char {
    let rep: &'a Replica = rep_ref(rep);
    if let Some(ref e) = rep.error {
        e.as_ptr()
    } else {
        std::ptr::null()
    }
}

/// Free a Replica.
#[no_mangle]
pub extern "C" fn tc_replica_free(rep: *mut Replica) {
    drop(unsafe { Box::from_raw(rep) });
}

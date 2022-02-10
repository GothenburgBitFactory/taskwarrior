use crate::task::TCTask;
use crate::traits::*;
use std::ptr::NonNull;

/// TCTaskList represents a list of tasks.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCTaskList {
    /// number of tasks in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// array of pointers representing each task. these remain owned by the TCTaskList instance and
    /// will be freed by tc_task_list_free.  This pointer is never NULL for a valid TCTaskList,
    /// and the *TCTaskList at indexes 0..len-1 are not NULL.
    items: *const NonNull<TCTask>,
}

impl PointerArray for TCTaskList {
    type Element = TCTask;

    unsafe fn from_raw_parts(items: *const NonNull<Self::Element>, len: usize, cap: usize) -> Self {
        TCTaskList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const NonNull<Self::Element>, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCTaskList instance.  The instance, and all TCTaskList it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCTaskList.
#[no_mangle]
pub unsafe extern "C" fn tc_task_list_free(tctasks: *mut TCTaskList) {
    debug_assert!(!tctasks.is_null());
    // SAFETY:
    //  - *tctasks is a valid TCTaskList (caller promises to treat it as read-only)
    let tasks = unsafe { TCTaskList::take_from_arg(tctasks, TCTaskList::null_value()) };
    TCTaskList::drop_pointer_vector(tasks);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_array_has_non_null_pointer() {
        let tctasks = TCTaskList::return_val(Vec::new());
        assert!(!tctasks.items.is_null());
        assert_eq!(tctasks.len, 0);
        assert_eq!(tctasks._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tctasks = TCTaskList::return_val(Vec::new());
        // SAFETY: testing expected behavior
        unsafe { tc_task_list_free(&mut tctasks) };
        assert!(tctasks.items.is_null());
        assert_eq!(tctasks.len, 0);
        assert_eq!(tctasks._capacity, 0);
    }
}

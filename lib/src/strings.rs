use crate::string::TCString;
use crate::traits::*;
use crate::util::{drop_pointer_array, vec_into_raw_parts};
use std::ptr::NonNull;

/// TCStrings represents a list of tags associated with a task.
///
/// The content of this struct must be treated as read-only.
///
/// The lifetime of a TCStrings instance is independent of the task, and it
/// will remain valid even if the task is freed.
#[repr(C)]
pub struct TCStrings {
    /// number of tags in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// strings representing each tag. these remain owned by the TCStrings instance and will be freed
    /// by tc_strings_free.
    items: *const NonNull<TCString<'static>>,
}

impl PassByValue for Vec<NonNull<TCString<'static>>> {
    type CType = TCStrings;

    unsafe fn from_ctype(arg: TCStrings) -> Self {
        // SAFETY:
        //  - C treats TCStrings as read-only, so items, len, and _capacity all came
        //    from a Vec originally.
        unsafe { Vec::from_raw_parts(arg.items as *mut _, arg.len, arg._capacity) }
    }

    fn as_ctype(self) -> TCStrings {
        // emulate Vec::into_raw_parts():
        // - disable dropping the Vec with ManuallyDrop
        // - extract ptr, len, and capacity using those methods
        let (items, len, _capacity) = vec_into_raw_parts(self);
        TCStrings {
            len,
            _capacity,
            items,
        }
    }
}

impl Default for TCStrings {
    fn default() -> Self {
        Self {
            len: 0,
            _capacity: 0,
            items: std::ptr::null(),
        }
    }
}

/// Free a TCStrings instance.  The instance, and all TCStrings it contains, must not be used after
/// this call.
#[no_mangle]
pub extern "C" fn tc_strings_free<'a>(tctags: *mut TCStrings) {
    debug_assert!(!tctags.is_null());
    // SAFETY:
    //  - *tctags is a valid TCStrings (caller promises to treat it as read-only)
    let tags = unsafe { Vec::take_from_arg(tctags, TCStrings::default()) };
    drop_pointer_array(tags);
}

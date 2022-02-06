use crate::string::TCString;
use crate::traits::*;
use crate::util::{drop_pointer_array, vec_into_raw_parts};
use std::ptr::NonNull;

/// TCTags represents a list of tags associated with a task.
///
/// The content of this struct must be treated as read-only.
///
/// The lifetime of a TCTags instance is independent of the task, and it
/// will remain valid even if the task is freed.
#[repr(C)]
pub struct TCTags {
    /// number of tags in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// strings representing each tag. these remain owned by the TCTags instance and will be freed
    /// by tc_tags_free.
    items: *const NonNull<TCString<'static>>,
}

impl PassByValue for Vec<NonNull<TCString<'static>>> {
    type CType = TCTags;

    unsafe fn from_ctype(arg: TCTags) -> Self {
        // SAFETY:
        //  - C treats TCTags as read-only, so items, len, and _capacity all came
        //    from a Vec originally.
        unsafe { Vec::from_raw_parts(arg.items as *mut _, arg.len, arg._capacity) }
    }

    fn as_ctype(self) -> TCTags {
        // emulate Vec::into_raw_parts():
        // - disable dropping the Vec with ManuallyDrop
        // - extract ptr, len, and capacity using those methods
        let (items, len, _capacity) = vec_into_raw_parts(self);
        TCTags {
            len,
            _capacity,
            items,
        }
    }
}

impl Default for TCTags {
    fn default() -> Self {
        Self {
            len: 0,
            _capacity: 0,
            items: std::ptr::null(),
        }
    }
}

/// Free a TCTags instance.  The instance, and all TCStrings it contains, must not be used after
/// this call.
#[no_mangle]
pub extern "C" fn tc_tags_free<'a>(tctags: *mut TCTags) {
    debug_assert!(!tctags.is_null());
    // SAFETY:
    //  - *tctags is a valid TCTags (caller promises to treat it as read-only)
    let tags = unsafe { Vec::take_from_arg(tctags, TCTags::default()) };
    drop_pointer_array(tags);
}

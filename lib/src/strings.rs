use crate::string::TCString;
use crate::traits::*;
use crate::util::{drop_pointer_array, vec_into_raw_parts};
use std::ptr::NonNull;

/// TCStrings represents a list of strings.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCStrings {
    /// number of strings in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// strings representing each string. these remain owned by the TCStrings instance and will be freed
    /// by tc_strings_free.
    items: *const NonNull<TCString<'static>>,
}

impl PassByValue for TCStrings {
    type RustType = Vec<NonNull<TCString<'static>>>;

    unsafe fn from_ctype(self) -> Self::RustType {
        // SAFETY:
        //  - C treats TCStrings as read-only, so items, len, and _capacity all came
        //    from a Vec originally.
        unsafe { Vec::from_raw_parts(self.items as *mut _, self.len, self._capacity) }
    }

    fn as_ctype(arg: Self::RustType) -> Self {
        // emulate Vec::into_raw_parts():
        // - disable dropping the Vec with ManuallyDrop
        // - extract ptr, len, and capacity using those methods
        let (items, len, _capacity) = vec_into_raw_parts(arg);
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
pub extern "C" fn tc_strings_free<'a>(tcstrings: *mut TCStrings) {
    debug_assert!(!tcstrings.is_null());
    // SAFETY:
    //  - *tcstrings is a valid TCStrings (caller promises to treat it as read-only)
    let strings = unsafe { TCStrings::take_from_arg(tcstrings, TCStrings::default()) };
    drop_pointer_array(strings);
}

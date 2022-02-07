use crate::string::TCString;
use crate::traits::*;
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

    /// TCStrings representing each string. these remain owned by the TCStrings instance and will
    /// be freed by tc_strings_free.  This pointer is never NULL for a valid TCStrings, and the
    /// *TCStrings at indexes 0..len-1 are not NULL.
    items: *const NonNull<TCString<'static>>,
}

impl PointerArray for TCStrings {
    type Element = TCString<'static>;

    unsafe fn from_raw_parts(items: *const NonNull<Self::Element>, len: usize, cap: usize) -> Self {
        TCStrings {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const NonNull<Self::Element>, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCStrings instance.  The instance, and all TCStrings it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCStrings.
#[no_mangle]
pub extern "C" fn tc_strings_free(tcstrings: *mut TCStrings) {
    debug_assert!(!tcstrings.is_null());
    // SAFETY:
    //  - *tcstrings is a valid TCStrings (caller promises to treat it as read-only)
    let strings = unsafe { TCStrings::take_from_arg(tcstrings, TCStrings::null_value()) };
    TCStrings::drop_pointer_vector(strings);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_array_has_non_null_pointer() {
        let tcstrings = TCStrings::return_val(Vec::new());
        assert!(!tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcstrings = TCStrings::return_val(Vec::new());
        tc_strings_free(&mut tcstrings);
        assert!(tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }
}

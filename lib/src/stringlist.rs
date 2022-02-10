use crate::traits::*;
use crate::types::*;
use std::ptr::NonNull;

/// TCStringList represents a list of strings.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCStringList {
    /// number of strings in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// TCStringList representing each string. these remain owned by the TCStringList instance and will
    /// be freed by tc_string_list_free.  This pointer is never NULL for a valid TCStringList, and the
    /// *TCStringList at indexes 0..len-1 are not NULL.
    items: *const NonNull<TCString<'static>>,
}

impl ValueArray for TCStringList {
    type Element = NonNull<TCString<'static>>;

    unsafe fn from_raw_parts(items: *const Self::Element, len: usize, cap: usize) -> Self {
        TCStringList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const Self::Element, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCStringList instance.  The instance, and all TCStringList it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCStringList.
#[no_mangle]
pub unsafe extern "C" fn tc_string_list_free(tcstrings: *mut TCStringList) {
    debug_assert!(!tcstrings.is_null());
    // SAFETY:
    //  - *tcstrings is a valid TCStringList (caller promises to treat it as read-only)
    let strings = unsafe { TCStringList::take_from_arg(tcstrings, TCStringList::null_value()) };
    TCStringList::drop_vector(strings);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_array_has_non_null_pointer() {
        let tcstrings = TCStringList::return_val(Vec::new());
        assert!(!tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcstrings = TCStringList::return_val(Vec::new());
        // SAFETY: testing expected behavior
        unsafe { tc_string_list_free(&mut tcstrings) };
        assert!(tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }
}

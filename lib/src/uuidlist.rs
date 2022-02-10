use crate::traits::*;
use crate::types::*;

/// TCUuidList represents a list of uuids.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCUuidList {
    /// number of uuids in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// array of uuids. these remain owned by the TCUuidList instance and will be freed by
    /// tc_uuid_list_free.  This pointer is never NULL for a valid TCUuidList.
    items: *const TCUuid,
}

impl ValueArray for TCUuidList {
    type Element = TCUuid;

    unsafe fn from_raw_parts(items: *const Self::Element, len: usize, cap: usize) -> Self {
        TCUuidList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const Self::Element, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCUuidList instance.  The instance, and all TCUuids it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCUuidList.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_list_free(tcuuids: *mut TCUuidList) {
    debug_assert!(!tcuuids.is_null());
    // SAFETY:
    //  - *tcuuids is a valid TCUuidList (caller promises to treat it as read-only)
    let uuids = unsafe { TCUuidList::take_from_arg(tcuuids, TCUuidList::null_value()) };
    TCUuidList::drop_vector(uuids);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_array_has_non_null_pointer() {
        let tcuuids = TCUuidList::return_val(Vec::new());
        assert!(!tcuuids.items.is_null());
        assert_eq!(tcuuids.len, 0);
        assert_eq!(tcuuids._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcuuids = TCUuidList::return_val(Vec::new());
        // SAFETY: testing expected behavior
        unsafe { tc_uuid_list_free(&mut tcuuids) };
        assert!(tcuuids.items.is_null());
        assert_eq!(tcuuids.len, 0);
        assert_eq!(tcuuids._capacity, 0);
    }
}

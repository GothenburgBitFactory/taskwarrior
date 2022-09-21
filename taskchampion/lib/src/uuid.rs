use crate::traits::*;
use crate::types::*;
use libc;
use taskchampion::Uuid;

// NOTE: this must be a simple constant so that cbindgen can evaluate it
/// Length, in bytes, of the string representation of a UUID (without NUL terminator)
pub const TC_UUID_STRING_BYTES: usize = 36;

/// TCUuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
/// Uuids are typically treated as opaque, but the bytes are available in big-endian format.
///
/// cbindgen:field-names=[bytes]
#[repr(C)]
pub struct TCUuid([u8; 16]);

impl PassByValue for TCUuid {
    type RustType = Uuid;

    unsafe fn from_ctype(self) -> Self::RustType {
        // SAFETY:
        //  - any 16-byte value is a valid Uuid
        Uuid::from_bytes(self.0)
    }

    fn as_ctype(arg: Uuid) -> Self {
        TCUuid(*arg.as_bytes())
    }
}

/// Create a new, randomly-generated UUID.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_new_v4() -> TCUuid {
    // SAFETY:
    // - value is not allocated
    unsafe { TCUuid::return_val(Uuid::new_v4()) }
}

/// Create a new UUID with the nil value.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_nil() -> TCUuid {
    // SAFETY:
    // - value is not allocated
    unsafe { TCUuid::return_val(Uuid::nil()) }
}

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
    items: *mut TCUuid,
}

impl CList for TCUuidList {
    type Element = TCUuid;

    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self {
        TCUuidList {
            len,
            _capacity: cap,
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
        (self.items, self.len, self._capacity)
    }
}

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_to_buf(tcuuid: TCUuid, buf: *mut libc::c_char) {
    debug_assert!(!buf.is_null());
    // SAFETY:
    //  - buf is valid for len bytes (by C convention)
    //  - (no alignment requirements for a byte slice)
    //  - content of buf will not be mutated during the lifetime of this slice (lifetime
    //    does not outlive this function call)
    //  - the length of the buffer is less than isize::MAX (promised by caller)
    let buf: &mut [u8] =
        unsafe { std::slice::from_raw_parts_mut(buf as *mut u8, TC_UUID_STRING_BYTES) };
    // SAFETY:
    //  - tcuuid is a valid TCUuid (all byte patterns are valid)
    let uuid: Uuid = unsafe { TCUuid::val_from_arg(tcuuid) };
    uuid.as_hyphenated().encode_lower(buf);
}

/// Return the hyphenated string representation of a TCUuid.  The returned string
/// must be freed with tc_string_free.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_to_str(tcuuid: TCUuid) -> TCString {
    // SAFETY:
    //  - tcuuid is a valid TCUuid (all byte patterns are valid)
    let uuid: Uuid = unsafe { TCUuid::val_from_arg(tcuuid) };
    let s = uuid.to_string();
    // SAFETY:
    //  - caller promises to free this value.
    unsafe { TCString::return_val(s.into()) }
}

/// Parse the given string as a UUID.  Returns TC_RESULT_ERROR on parse failure or if the given
/// string is not valid.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_from_str(s: TCString, uuid_out: *mut TCUuid) -> TCResult {
    debug_assert!(!s.is_null());
    debug_assert!(!uuid_out.is_null());
    // SAFETY:
    //  - s is valid (promised by caller)
    //  - caller will not use s after this call (convention)
    let mut s = unsafe { TCString::val_from_arg(s) };
    if let Ok(s) = s.as_str() {
        if let Ok(u) = Uuid::parse_str(s) {
            // SAFETY:
            //  - uuid_out is not NULL (promised by caller)
            //  - alignment is not required
            unsafe { TCUuid::val_to_arg_out(u, uuid_out) };
            return TCResult::Ok;
        }
    }
    TCResult::Error
}

/// Free a TCUuidList instance.  The instance, and all TCUuids it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCUuidList.
#[no_mangle]
pub unsafe extern "C" fn tc_uuid_list_free(tcuuids: *mut TCUuidList) {
    // SAFETY:
    //  - tcuuids is not NULL and points to a valid TCUuidList (caller is not allowed to
    //    modify the list)
    //  - caller promises not to use the value after return
    unsafe { drop_value_list(tcuuids) };
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_list_has_non_null_pointer() {
        let tcuuids = unsafe { TCUuidList::return_val(Vec::new()) };
        assert!(!tcuuids.items.is_null());
        assert_eq!(tcuuids.len, 0);
        assert_eq!(tcuuids._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcuuids = unsafe { TCUuidList::return_val(Vec::new()) };
        // SAFETY: testing expected behavior
        unsafe { tc_uuid_list_free(&mut tcuuids) };
        assert!(tcuuids.items.is_null());
        assert_eq!(tcuuids.len, 0);
        assert_eq!(tcuuids._capacity, 0);
    }
}

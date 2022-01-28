use crate::string::TCString;
use libc;
use taskchampion::Uuid;

/// TCUuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
/// Uuids are typically treated as opaque, but the bytes are available in big-endian format.
///
/// cbindgen:field-names=[bytes]
#[repr(C)]
pub struct TCUuid([u8; 16]);

impl From<Uuid> for TCUuid {
    fn from(uuid: Uuid) -> TCUuid {
        TCUuid(*uuid.as_bytes())
    }
}

impl From<TCUuid> for Uuid {
    fn from(uuid: TCUuid) -> Uuid {
        Uuid::from_bytes(uuid.0)
    }
}

/// Create a new, randomly-generated UUID.
#[no_mangle]
pub extern "C" fn tc_uuid_new_v4() -> TCUuid {
    Uuid::new_v4().into()
}

/// Create a new UUID with the nil value.
#[no_mangle]
pub extern "C" fn tc_uuid_nil() -> TCUuid {
    Uuid::nil().into()
}

/// Length, in bytes, of a C string containing a TCUuid.
// TODO: why not a const?
#[no_mangle]
pub static TC_UUID_STRING_BYTES: usize = ::uuid::adapter::Hyphenated::LENGTH;

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
#[no_mangle]
pub extern "C" fn tc_uuid_to_buf<'a>(uuid: TCUuid, buf: *mut libc::c_char) {
    debug_assert!(!buf.is_null());
    // SAFETY:
    //  - buf is valid for len bytes (by C convention)
    //  - (no alignment requirements for a byte slice)
    //  - content of buf will not be mutated during the lifetime of this slice (lifetime
    //    does not outlive this function call)
    //  - the length of the buffer is less than isize::MAX (promised by caller)
    let buf: &'a mut [u8] = unsafe {
        std::slice::from_raw_parts_mut(buf as *mut u8, ::uuid::adapter::Hyphenated::LENGTH)
    };
    let uuid: Uuid = uuid.into();
    uuid.to_hyphenated().encode_lower(buf);
}

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
#[no_mangle]
pub extern "C" fn tc_uuid_to_str(uuid: TCUuid) -> *mut TCString<'static> {
    let uuid: Uuid = uuid.into();
    let s = uuid.to_string();
    TCString::from(s).return_val()
}

/// Parse the given string as a UUID.  The string must not be NULL.  Returns false on failure.
#[no_mangle]
pub extern "C" fn tc_uuid_from_str<'a>(s: *mut TCString, uuid_out: *mut TCUuid) -> bool {
    debug_assert!(!s.is_null());
    debug_assert!(!uuid_out.is_null());
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (implicitly promised by caller)
    let s = unsafe { TCString::from_arg(s) };
    if let Ok(s) = s.as_str() {
        if let Ok(u) = Uuid::parse_str(s) {
            // SAFETY:
            //  - uuid_out is not NULL (promised by caller)
            //  - alignment is not required
            unsafe { *uuid_out = u.into() };
            return true;
        }
    }
    false
}

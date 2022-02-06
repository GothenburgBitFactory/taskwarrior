use crate::string::TCString;
use crate::traits::*;
use libc;
use taskchampion::Uuid;

/// TCUuid is used as a task identifier.  Uuids do not contain any pointers and need not be freed.
/// Uuids are typically treated as opaque, but the bytes are available in big-endian format.
///
/// cbindgen:field-names=[bytes]
#[repr(C)]
pub struct TCUuid([u8; 16]);

impl PassByValue for Uuid {
    type CType = TCUuid;

    unsafe fn from_ctype(arg: TCUuid) -> Self {
        // SAFETY:
        //  - any 16-byte value is a valid Uuid
        Uuid::from_bytes(arg.0)
    }

    fn as_ctype(self) -> TCUuid {
        TCUuid(*self.as_bytes())
    }
}

/// Create a new, randomly-generated UUID.
#[no_mangle]
pub extern "C" fn tc_uuid_new_v4() -> TCUuid {
    Uuid::new_v4().return_val()
}

/// Create a new UUID with the nil value.
#[no_mangle]
pub extern "C" fn tc_uuid_nil() -> TCUuid {
    Uuid::nil().return_val()
}

// NOTE: this must be a simple constant so that cbindgen can evaluate it
/// Length, in bytes, of the string representation of a UUID (without NUL terminator)
pub const TC_UUID_STRING_BYTES: usize = 36;

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
#[no_mangle]
pub extern "C" fn tc_uuid_to_buf<'a>(tcuuid: TCUuid, buf: *mut libc::c_char) {
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
    let uuid: Uuid = Uuid::from_arg(tcuuid);
    uuid.to_hyphenated().encode_lower(buf);
}

/// Write the string representation of a TCUuid into the given buffer, which must be
/// at least TC_UUID_STRING_BYTES long.  No NUL terminator is added.
#[no_mangle]
pub extern "C" fn tc_uuid_to_str(tcuuid: TCUuid) -> *mut TCString<'static> {
    let uuid: Uuid = Uuid::from_arg(tcuuid);
    let s = uuid.to_string();
    // SAFETY: see TCString docstring
    unsafe { TCString::from(s).return_val() }
}

/// Parse the given string as a UUID.  Returns false on failure.
#[no_mangle]
pub extern "C" fn tc_uuid_from_str<'a>(s: *mut TCString, uuid_out: *mut TCUuid) -> bool {
    // TODO: TCResult instead
    debug_assert!(!s.is_null());
    debug_assert!(!uuid_out.is_null());
    // SAFETY: see TCString docstring
    let s = unsafe { TCString::take_from_arg(s) };
    if let Ok(s) = s.as_str() {
        if let Ok(u) = Uuid::parse_str(s) {
            // SAFETY:
            //  - uuid_out is not NULL (promised by caller)
            //  - alignment is not required
            unsafe { u.to_arg_out(uuid_out) };
            return true;
        }
    }
    false
}

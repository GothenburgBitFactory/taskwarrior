use std::ffi::{CStr, CString, OsStr};
use std::os::unix::ffi::OsStrExt;
use std::path::PathBuf;

/// TCString supports passing strings into and out of the TaskChampion API.
///
/// Unless specified otherwise, functions in this API take ownership of a TCString when it appears
/// as a function argument, and transfer ownership to the caller when the TCString appears as a
/// return value or otput argument.
pub enum TCString<'a> {
    CString(CString),
    CStr(&'a CStr),
    String(String),
}

impl<'a> TCString<'a> {
    /// Take a TCString from C as an argument.  C callers generally expect TC functions to take
    /// ownership of a string, which is what this function does.
    pub(crate) fn from_arg(tcstring: *mut TCString<'a>) -> Self {
        debug_assert!(!tcstring.is_null());
        *(unsafe { Box::from_raw(tcstring) })
    }

    /// Borrow a TCString from C as an argument.
    pub(crate) fn from_arg_ref(tcstring: *mut TCString<'a>) -> &'a mut Self {
        debug_assert!(!tcstring.is_null());
        unsafe { &mut *tcstring }
    }

    /// Get a regular Rust &str for this value.
    pub(crate) fn as_str(&self) -> Result<&str, std::str::Utf8Error> {
        match self {
            TCString::CString(cstring) => cstring.as_c_str().to_str(),
            TCString::CStr(cstr) => cstr.to_str(),
            TCString::String(string) => Ok(string.as_ref()),
        }
    }

    pub(crate) fn as_bytes(&self) -> &[u8] {
        match self {
            TCString::CString(cstring) => cstring.as_bytes(),
            TCString::CStr(cstr) => cstr.to_bytes(),
            TCString::String(string) => string.as_bytes(),
        }
    }

    pub(crate) fn to_path_buf(&self) -> PathBuf {
        // TODO: this is UNIX-specific.
        let path: &OsStr = OsStr::from_bytes(self.as_bytes());
        path.to_os_string().into()
    }

    /// Convert this to a return value for handing off to C.
    pub(crate) fn return_val(self) -> *mut TCString<'a> {
        Box::into_raw(Box::new(self))
    }
}

impl<'a> From<String> for TCString<'a> {
    fn from(string: String) -> TCString<'a> {
        TCString::String(string)
    }
}

impl<'a> From<&str> for TCString<'a> {
    fn from(string: &str) -> TCString<'a> {
        TCString::String(string.to_string())
    }
}

/// Create a new TCString referencing the given C string.  The C string must remain valid until
/// after the TCString is freed.  It's typically easiest to ensure this by using a static string.
#[no_mangle]
pub extern "C" fn tc_string_new(cstr: *const libc::c_char) -> *mut TCString<'static> {
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    TCString::CStr(cstr).return_val()
}

/// Create a new TCString by cloning the content of the given C string.
#[no_mangle]
pub extern "C" fn tc_string_clone(cstr: *const libc::c_char) -> *mut TCString<'static> {
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    TCString::CString(cstr.into()).return_val()
}

/// Create a new TCString containing the given string with the given length. This allows creation
/// of strings containing embedded NUL characters.  If the given string is not valid UTF-8, this
/// function will return NULL.
#[no_mangle]
pub extern "C" fn tc_string_clone_with_len(
    buf: *const libc::c_char,
    len: usize,
) -> *mut TCString<'static> {
    let slice = unsafe { std::slice::from_raw_parts(buf as *const u8, len) };
    let vec = slice.to_vec();
    if let Ok(string) = String::from_utf8(vec) {
        TCString::String(string).return_val()
    } else {
        std::ptr::null_mut()
    }
}

/// Get the content of the string as a regular C string.  The given string must not be NULL.  The
/// returned value may be NULL if the string contains NUL bytes.
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub extern "C" fn tc_string_content(tcstring: *mut TCString) -> *const libc::c_char {
    let tcstring = TCString::from_arg_ref(tcstring);
    // if we have a String, we need to consume it and turn it into
    // a CString.
    if let TCString::String(string) = tcstring {
        // TODO: get rid of this clone
        match CString::new(string.clone()) {
            Ok(cstring) => {
                *tcstring = TCString::CString(cstring);
            }
            Err(_) => {
                // TODO: could recover the underlying String
                return std::ptr::null();
            }
        }
    }

    match tcstring {
        TCString::CString(cstring) => cstring.as_ptr(),
        TCString::String(_) => unreachable!(), // just converted this
        TCString::CStr(cstr) => cstr.as_ptr(),
    }
}

/// Free a TCString.
#[no_mangle]
pub extern "C" fn tc_string_free(string: *mut TCString) {
    debug_assert!(!string.is_null());
    drop(unsafe { Box::from_raw(string) });
}

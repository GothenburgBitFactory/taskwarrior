use std::ffi::{CStr, CString, NulError};

// thinking:
//  - TCString ownership always taken when passed in
//  - TCString ownership always falls to C when passed out
//  - accept that bytes must be copied to get owned string
//  - Can we do this with an enum of some sort?

/// TCString supports passing strings into and out of the TaskChampion API.
pub struct TCString(CString);

impl TCString {
    /// Take a TCString from C as an argument.
    pub(crate) fn from_arg(tcstring: *mut TCString) -> Self {
        debug_assert!(!tcstring.is_null());
        *(unsafe { Box::from_raw(tcstring) })
    }

    /// Get a regular Rust &str for this value.
    pub(crate) fn as_str(&self) -> Result<&str, std::str::Utf8Error> {
        self.0.as_c_str().to_str()
    }

    /// Construct a *mut TCString from a string for returning to C.
    pub(crate) fn return_string(string: impl Into<Vec<u8>>) -> Result<*mut TCString, NulError> {
        let tcstring = TCString(CString::new(string)?);
        Ok(Box::into_raw(Box::new(tcstring)))
    }
}

#[no_mangle]
pub extern "C" fn tc_string_new(cstr: *const libc::c_char) -> *mut TCString {
    let cstring = unsafe { CStr::from_ptr(cstr) }.into();
    Box::into_raw(Box::new(TCString(cstring)))
}

#[no_mangle]
pub extern "C" fn tc_string_content(string: *mut TCString) -> *const libc::c_char {
    debug_assert!(!string.is_null());
    let string: &CString = unsafe { &(*string).0 };
    string.as_ptr()
}

#[no_mangle]
pub extern "C" fn tc_string_free(string: *mut TCString) {
    debug_assert!(!string.is_null());
    drop(unsafe { Box::from_raw(string) });
}

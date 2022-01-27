use std::ffi::{CStr, CString, OsStr};
use std::os::unix::ffi::OsStrExt;
use std::path::PathBuf;
use std::str::Utf8Error;

/// TCString supports passing strings into and out of the TaskChampion API.  A string can contain
/// embedded NUL characters.  Strings containing such embedded NULs cannot be represented as a "C
/// string" and must be accessed using `tc_string_content_and_len` and `tc_string_clone_with_len`.
/// In general, these two functions should be used for handling arbitrary data, while more
/// convenient forms may be used where embedded NUL characters are impossible, such as in static
/// strings.
///
/// Rust expects all strings to be UTF-8, and API functions will fail if given a TCString
/// containing invalid UTF-8.
///
/// Unless specified otherwise, functions in this API take ownership of a TCString when it is given
/// as a function argument, and free the string before returning.
///
/// When a TCString appears as a return value or output argument, it is the responsibility of the
/// caller to free the string.
pub enum TCString<'a> {
    CString(CString),
    CStr(&'a CStr),
    String(String),

    /// This variant denotes an input string that was not valid UTF-8.  This allows reporting this
    /// error when the string is read, with the constructor remaining infallible.
    InvalidUtf8(Utf8Error, Vec<u8>),

    /// None is the default value for TCString, but this variant is never seen by C code or by Rust
    /// code outside of this module.
    None,
}

impl<'a> Default for TCString<'a> {
    fn default() -> Self {
        TCString::None
    }
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
            TCString::InvalidUtf8(e, _) => Err(*e),
            TCString::None => unreachable!(),
        }
    }

    fn as_bytes(&self) -> &[u8] {
        match self {
            TCString::CString(cstring) => cstring.as_bytes(),
            TCString::CStr(cstr) => cstr.to_bytes(),
            TCString::String(string) => string.as_bytes(),
            TCString::InvalidUtf8(_, data) => data.as_ref(),
            TCString::None => unreachable!(),
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
///
/// NOTE: this function does _not_ take responsibility for freeing the C string itself.  The
/// underlying string can be freed once the TCString referencing it has been freed.
///
/// For example:
///
/// ```
/// char *url = get_item_url(..); // dynamically allocate C string
/// tc_task_annotate(task, tc_string_borrow(url)); // TCString created, passed, and freed
/// free(url); // string is no longer referenced and can be freed
/// ```
#[no_mangle]
pub extern "C" fn tc_string_borrow(cstr: *const libc::c_char) -> *mut TCString<'static> {
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    TCString::CStr(cstr).return_val()
}

/// Create a new TCString by cloning the content of the given C string.  The resulting TCString
/// is independent of the given string, which can be freed or overwritten immediately.
#[no_mangle]
pub extern "C" fn tc_string_clone(cstr: *const libc::c_char) -> *mut TCString<'static> {
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    TCString::CString(cstr.into()).return_val()
}

/// Create a new TCString containing the given string with the given length. This allows creation
/// of strings containing embedded NUL characters.  As with `tc_string_clone`, the resulting
/// TCString is independent of the passed buffer, which may be reused or freed immediately.
#[no_mangle]
pub extern "C" fn tc_string_clone_with_len(
    buf: *const libc::c_char,
    len: usize,
) -> *mut TCString<'static> {
    let slice = unsafe { std::slice::from_raw_parts(buf as *const u8, len) };
    let vec = slice.to_vec();
    // try converting to a string, which is the only variant that can contain embedded NULs.  If
    // the bytes are not valid utf-8, store that information for reporting later.
    match String::from_utf8(vec) {
        Ok(string) => TCString::String(string),
        Err(e) => {
            let (e, vec) = (e.utf8_error(), e.into_bytes());
            TCString::InvalidUtf8(e, vec)
        }
    }
    .return_val()
}

/// Get the content of the string as a regular C string.  The given string must not be NULL.  The
/// returned value is NULL if the string contains NUL bytes or (in some cases) invalid UTF-8.  The
/// returned C string is valid until the TCString is freed or passed to another TC API function.
///
/// In general, prefer [`tc_string_content_with_len`] except when it's certain that the string is
/// valid and NUL-free.
///
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub extern "C" fn tc_string_content(tcstring: *mut TCString) -> *const libc::c_char {
    let tcstring = TCString::from_arg_ref(tcstring);

    // if we have a String, we need to consume it and turn it into
    // a CString.
    if matches!(tcstring, TCString::String(_)) {
        if let TCString::String(string) = std::mem::take(tcstring) {
            match CString::new(string) {
                Ok(cstring) => {
                    *tcstring = TCString::CString(cstring);
                }
                Err(nul_err) => {
                    // recover the underlying String from the NulError
                    let original_bytes = nul_err.into_vec();
                    // SAFETY: original_bytes came from a String moments ago, so still valid utf8
                    let string = unsafe { String::from_utf8_unchecked(original_bytes) };
                    *tcstring = TCString::String(string);

                    // and return NULL as advertized
                    return std::ptr::null();
                }
            }
        } else {
            unreachable!()
        }
    }

    match tcstring {
        TCString::CString(cstring) => cstring.as_ptr(),
        TCString::String(_) => unreachable!(), // just converted to CString
        TCString::CStr(cstr) => cstr.as_ptr(),
        TCString::InvalidUtf8(_, _) => std::ptr::null(),
        TCString::None => unreachable!(),
    }
}

/// Get the content of the string as a pointer and length.  The given string must not be NULL.
/// This function can return any string, even one including NUL bytes or invalid UTF-8.  The
/// returned buffer is valid until the TCString is freed or passed to another TC API function.
///
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub extern "C" fn tc_string_content_with_len(
    tcstring: *mut TCString,
    len_out: *mut usize,
) -> *const libc::c_char {
    debug_assert!(!tcstring.is_null());
    debug_assert!(!len_out.is_null());
    let tcstring = TCString::from_arg_ref(tcstring);
    let bytes = match tcstring {
        TCString::CString(cstring) => cstring.as_bytes(),
        TCString::String(string) => string.as_bytes(),
        TCString::CStr(cstr) => cstr.to_bytes(),
        TCString::InvalidUtf8(_, ref v) => v.as_ref(),
        TCString::None => unreachable!(),
    };
    unsafe { *len_out = bytes.len() };
    bytes.as_ptr() as *const libc::c_char
}

/// Free a TCString.
#[no_mangle]
pub extern "C" fn tc_string_free(string: *mut TCString) {
    debug_assert!(!string.is_null());
    drop(unsafe { Box::from_raw(string) });
}

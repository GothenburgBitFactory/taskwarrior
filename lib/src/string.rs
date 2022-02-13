use crate::traits::*;
use std::ffi::{CStr, CString, OsStr};
use std::os::unix::ffi::OsStrExt;
use std::path::PathBuf;
use std::ptr::NonNull;
use std::str::Utf8Error;

/// TCString supports passing strings into and out of the TaskChampion API.
///
/// # Rust Strings and C Strings
///
/// A Rust string can contain embedded NUL characters, while C considers such a character to mark
/// the end of a string.  Strings containing embedded NULs cannot be represented as a "C string"
/// and must be accessed using `tc_string_content_and_len` and `tc_string_clone_with_len`.  In
/// general, these two functions should be used for handling arbitrary data, while more convenient
/// forms may be used where embedded NUL characters are impossible, such as in static strings.
///
/// # UTF-8
///
/// TaskChampion expects all strings to be valid UTF-8. `tc_string_…` functions will fail if given
/// a `*TCString` containing invalid UTF-8.
///
/// # Safety
///
/// When a `*TCString` appears as a return value or output argument, ownership is passed to the
/// caller.  The caller must pass that ownerhsip back to another function or free the string.
///
/// Any function taking a `*TCReplica` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function; and
///  - the memory referenced by the pointer must never be modified by C code.
///
/// Unless specified otherwise, TaskChampion functions take ownership of a `*TCString` when it is
/// given as a function argument, and the pointer is invalid when the function returns.  Callers
/// must not use or free TCStringList after passing them to such API functions.
///
/// TCString is not threadsafe.
#[derive(PartialEq, Debug)]
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

impl<'a> PassByPointer for TCString<'a> {}

impl<'a> TCString<'a> {
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

    /// Consume this TCString and return an equivalent String, or an error if not
    /// valid UTF-8.  In the error condition, the original data is lost.
    pub(crate) fn into_string(self) -> Result<String, std::str::Utf8Error> {
        match self {
            TCString::CString(cstring) => cstring.into_string().map_err(|e| e.utf8_error()),
            TCString::CStr(cstr) => cstr.to_str().map(|s| s.to_string()),
            TCString::String(string) => Ok(string),
            TCString::InvalidUtf8(e, _) => Err(e),
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

    /// Convert the TCString, in place, into one of the C variants.  If this is not
    /// possible, such as if the string contains an embedded NUL, then the string
    /// remains unchanged.
    fn to_c_string_mut(&mut self) {
        if matches!(self, TCString::String(_)) {
            // we must take ownership of the String in order to try converting it,
            // leaving the underlying TCString as its default (None)
            if let TCString::String(string) = std::mem::take(self) {
                match CString::new(string) {
                    Ok(cstring) => *self = TCString::CString(cstring),
                    Err(nul_err) => {
                        // recover the underlying String from the NulError and restore
                        // the TCString
                        let original_bytes = nul_err.into_vec();
                        // SAFETY: original_bytes came from a String moments ago, so still valid utf8
                        let string = unsafe { String::from_utf8_unchecked(original_bytes) };
                        *self = TCString::String(string);
                    }
                }
            } else {
                // the `matches!` above verified self was a TCString::String
                unreachable!()
            }
        }
    }

    pub(crate) fn to_path_buf(&self) -> PathBuf {
        // TODO: this is UNIX-specific.
        let path: &OsStr = OsStr::from_bytes(self.as_bytes());
        path.to_os_string().into()
    }
}

impl<'a> From<String> for TCString<'a> {
    fn from(string: String) -> TCString<'a> {
        TCString::String(string)
    }
}

impl<'a> From<&str> for TCString<'static> {
    fn from(string: &str) -> TCString<'static> {
        TCString::String(string.to_string())
    }
}

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

impl CList for TCStringList {
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

/// Create a new TCString referencing the given C string.  The C string must remain valid and
/// unchanged until after the TCString is freed.  It's typically easiest to ensure this by using a
/// static string.
///
/// NOTE: this function does _not_ take responsibility for freeing the given C string.  The
/// given string can be freed once the TCString referencing it has been freed.
///
/// For example:
///
/// ```
/// char *url = get_item_url(..); // dynamically allocate C string
/// tc_task_annotate(task, tc_string_borrow(url)); // TCString created, passed, and freed
/// free(url); // string is no longer referenced and can be freed
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_string_borrow(cstr: *const libc::c_char) -> *mut TCString<'static> {
    debug_assert!(!cstr.is_null());
    // SAFETY:
    //  - cstr is not NULL (promised by caller, verified by assertion)
    //  - cstr's lifetime exceeds that of the TCString (promised by caller)
    //  - cstr contains a valid NUL terminator (promised by caller)
    //  - cstr's content will not change before it is destroyed (promised by caller)
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    // SAFETY: see docstring
    unsafe { TCString::CStr(cstr).return_ptr() }
}

/// Create a new TCString by cloning the content of the given C string.  The resulting TCString
/// is independent of the given string, which can be freed or overwritten immediately.
#[no_mangle]
pub unsafe extern "C" fn tc_string_clone(cstr: *const libc::c_char) -> *mut TCString<'static> {
    debug_assert!(!cstr.is_null());
    // SAFETY:
    //  - cstr is not NULL (promised by caller, verified by assertion)
    //  - cstr's lifetime exceeds that of this function (by C convention)
    //  - cstr contains a valid NUL terminator (promised by caller)
    //  - cstr's content will not change before it is destroyed (by C convention)
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    // SAFETY: see docstring
    unsafe { TCString::CString(cstr.into()).return_ptr() }
}

/// Create a new TCString containing the given string with the given length. This allows creation
/// of strings containing embedded NUL characters.  As with `tc_string_clone`, the resulting
/// TCString is independent of the passed buffer, which may be reused or freed immediately.
///
/// The given length must be less than half the maximum value of usize.
#[no_mangle]
pub unsafe extern "C" fn tc_string_clone_with_len(
    buf: *const libc::c_char,
    len: usize,
) -> *mut TCString<'static> {
    debug_assert!(!buf.is_null());
    debug_assert!(len < isize::MAX as usize);
    // SAFETY:
    //  - buf is valid for len bytes (by C convention)
    //  - (no alignment requirements for a byte slice)
    //  - content of buf will not be mutated during the lifetime of this slice (lifetime
    //    does not outlive this function call)
    //  - the length of the buffer is less than isize::MAX (promised by caller)
    let slice = unsafe { std::slice::from_raw_parts(buf as *const u8, len) };

    // allocate and copy into Rust-controlled memory
    let vec = slice.to_vec();

    // try converting to a string, which is the only variant that can contain embedded NULs.  If
    // the bytes are not valid utf-8, store that information for reporting later.
    let tcstring = match String::from_utf8(vec) {
        Ok(string) => TCString::String(string),
        Err(e) => {
            let (e, vec) = (e.utf8_error(), e.into_bytes());
            TCString::InvalidUtf8(e, vec)
        }
    };

    // SAFETY: see docstring
    unsafe { tcstring.return_ptr() }
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
pub unsafe extern "C" fn tc_string_content(tcstring: *mut TCString) -> *const libc::c_char {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - lifetime of tcstring outlives the lifetime of this function
    //  - lifetime of tcstring outlives the lifetime of the returned pointer (promised by caller)
    let tcstring = unsafe { TCString::from_ptr_arg_ref_mut(tcstring) };

    // if we have a String, we need to consume it and turn it into
    // a CString.
    tcstring.to_c_string_mut();

    match tcstring {
        TCString::CString(cstring) => cstring.as_ptr(),
        TCString::String(_) => std::ptr::null(), // to_c_string_mut failed
        TCString::CStr(cstr) => cstr.as_ptr(),
        TCString::InvalidUtf8(_, _) => std::ptr::null(),
        TCString::None => unreachable!(),
    }
}

/// Get the content of the string as a pointer and length.  The given string must not be NULL.
/// This function can return any string, even one including NUL bytes or invalid UTF-8.  The
/// returned buffer is valid until the TCString is freed or passed to another TaskChampio
/// function.
///
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub unsafe extern "C" fn tc_string_content_with_len(
    tcstring: *mut TCString,
    len_out: *mut usize,
) -> *const libc::c_char {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - lifetime of tcstring outlives the lifetime of this function
    //  - lifetime of tcstring outlives the lifetime of the returned pointer (promised by caller)
    let tcstring = unsafe { TCString::from_ptr_arg_ref(tcstring) };

    let bytes = tcstring.as_bytes();

    // SAFETY:
    //  - len_out is not NULL (promised by caller)
    //  - len_out points to valid memory (promised by caller)
    //  - len_out is properly aligned (C convention)
    unsafe { usize::val_to_arg_out(bytes.len(), len_out) };
    bytes.as_ptr() as *const libc::c_char
}

/// Free a TCString.  The given string must not be NULL.  The string must not be used
/// after this function returns, and must not be freed more than once.
#[no_mangle]
pub unsafe extern "C" fn tc_string_free(tcstring: *mut TCString) {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (promised by caller)
    drop(unsafe { TCString::take_from_ptr_arg(tcstring) });
}

/// Free a TCStringList instance.  The instance, and all TCStringList it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCStringList.
#[no_mangle]
pub unsafe extern "C" fn tc_string_list_free(tcstrings: *mut TCStringList) {
    // SAFETY:
    //  - tcstrings is not NULL and points to a valid TCStringList (caller is not allowed to
    //    modify the list)
    //  - caller promises not to use the value after return
    unsafe { drop_pointer_list(tcstrings) };
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn empty_list_has_non_null_pointer() {
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

    const INVALID_UTF8: &[u8] = b"abc\xf0\x28\x8c\x28";

    fn make_cstring() -> TCString<'static> {
        TCString::CString(CString::new("a string").unwrap())
    }

    fn make_cstr() -> TCString<'static> {
        let cstr = CStr::from_bytes_with_nul(b"a string\0").unwrap();
        TCString::CStr(&cstr)
    }

    fn make_string() -> TCString<'static> {
        TCString::String("a string".into())
    }

    fn make_string_with_nul() -> TCString<'static> {
        TCString::String("a \0 nul!".into())
    }

    fn make_invalid() -> TCString<'static> {
        let e = String::from_utf8(INVALID_UTF8.to_vec()).unwrap_err();
        TCString::InvalidUtf8(e.utf8_error(), e.into_bytes())
    }

    #[test]
    fn cstring_as_str() {
        assert_eq!(make_cstring().as_str().unwrap(), "a string");
    }

    #[test]
    fn cstr_as_str() {
        assert_eq!(make_cstr().as_str().unwrap(), "a string");
    }

    #[test]
    fn string_as_str() {
        assert_eq!(make_string().as_str().unwrap(), "a string");
    }

    #[test]
    fn string_with_nul_as_str() {
        assert_eq!(make_string_with_nul().as_str().unwrap(), "a \0 nul!");
    }

    #[test]
    fn invalid_as_str() {
        let as_str_err = make_invalid().as_str().unwrap_err();
        assert_eq!(as_str_err.valid_up_to(), 3); // "abc" is valid
    }

    #[test]
    fn cstring_as_bytes() {
        assert_eq!(make_cstring().as_bytes(), b"a string");
    }

    #[test]
    fn cstr_as_bytes() {
        assert_eq!(make_cstr().as_bytes(), b"a string");
    }

    #[test]
    fn string_as_bytes() {
        assert_eq!(make_string().as_bytes(), b"a string");
    }

    #[test]
    fn string_with_nul_as_bytes() {
        assert_eq!(make_string_with_nul().as_bytes(), b"a \0 nul!");
    }

    #[test]
    fn invalid_as_bytes() {
        assert_eq!(make_invalid().as_bytes(), INVALID_UTF8);
    }

    #[test]
    fn cstring_to_c_string_mut() {
        let mut tcstring = make_cstring();
        tcstring.to_c_string_mut();
        assert_eq!(tcstring, make_cstring()); // unchanged
    }

    #[test]
    fn cstr_to_c_string_mut() {
        let mut tcstring = make_cstr();
        tcstring.to_c_string_mut();
        assert_eq!(tcstring, make_cstr()); // unchanged
    }

    #[test]
    fn string_to_c_string_mut() {
        let mut tcstring = make_string();
        tcstring.to_c_string_mut();
        assert_eq!(tcstring, make_cstring()); // converted to CString, same content
    }

    #[test]
    fn string_with_nul_to_c_string_mut() {
        let mut tcstring = make_string_with_nul();
        tcstring.to_c_string_mut();
        assert_eq!(tcstring, make_string_with_nul()); // unchanged
    }

    #[test]
    fn invalid_to_c_string_mut() {
        let mut tcstring = make_invalid();
        tcstring.to_c_string_mut();
        assert_eq!(tcstring, make_invalid()); // unchanged
    }
}

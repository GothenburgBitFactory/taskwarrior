use crate::traits::*;
use crate::util::{string_into_raw_parts, vec_into_raw_parts};
use std::ffi::{CStr, CString, OsString};
use std::os::raw::c_char;
use std::path::PathBuf;

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
/// The `ptr` field may be checked for NULL, where documentation indicates this is possible.  All
/// other fields in a TCString are private and must not be used from C.  They exist in the struct
/// to ensure proper allocation and alignment.
///
/// When a `TCString` appears as a return value or output argument, ownership is passed to the
/// caller.  The caller must pass that ownership back to another function or free the string.
///
/// Any function taking a `TCString` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function; and
///  - the memory referenced by the pointer must never be modified by C code.
///
/// Unless specified otherwise, TaskChampion functions take ownership of a `TCString` when it is
/// given as a function argument, and the caller must not use or free TCStrings after passing them
/// to such API functions.
///
/// A TCString with a NULL `ptr` field need not be freed, although tc_free_string will not fail
/// for such a value.
///
/// TCString is not threadsafe.
/// cbindgen:field-names=[ptr, _u1, _u2, _u3]
#[repr(C)]
pub struct TCString {
    // defined based on the type
    ptr: *mut libc::c_void,
    len: usize,
    cap: usize,

    // type of TCString this represents
    ty: u8,
}

// TODO: figure out how to ignore this but still use it in TCString
/// A discriminator for TCString
#[repr(u8)]
enum TCStringType {
    /// Null.  Nothing is contained in this string.
    ///
    /// * `ptr` is NULL.
    /// * `len` and `cap` are zero.
    Null = 0,

    /// A CString.
    ///
    /// * `ptr` is the result of CString::into_raw, containing a terminating NUL.  It may not be
    ///   valid UTF-8.
    /// * `len` and `cap` are zero.
    CString,

    /// A CStr, referencing memory borrowed from C
    ///
    /// * `ptr` points to the string, containing a terminating NUL.  It may not be valid UTF-8.
    /// * `len` and `cap` are zero.
    CStr,

    /// A String.
    ///
    /// * `ptr`, `len`, and `cap` are as would be returned from String::into_raw_parts.
    String,

    /// A byte sequence.
    ///
    /// * `ptr`, `len`, and `cap` are as would be returned from Vec::into_raw_parts.
    Bytes,
}

impl Default for TCString {
    fn default() -> Self {
        TCString {
            ptr: std::ptr::null_mut(),
            len: 0,
            cap: 0,
            ty: TCStringType::Null as u8,
        }
    }
}

impl TCString {
    pub(crate) fn is_null(&self) -> bool {
        self.ptr.is_null()
    }
}

#[derive(PartialEq, Debug)]
pub enum RustString<'a> {
    Null,
    CString(CString),
    CStr(&'a CStr),
    String(String),
    Bytes(Vec<u8>),
}

impl<'a> Default for RustString<'a> {
    fn default() -> Self {
        RustString::Null
    }
}

impl PassByValue for TCString {
    type RustType = RustString<'static>;

    unsafe fn from_ctype(self) -> Self::RustType {
        match self.ty {
            ty if ty == TCStringType::CString as u8 => {
                // SAFETY:
                //  - ptr was derived from CString::into_raw
                //  - data was not modified since that time (caller promises)
                RustString::CString(unsafe { CString::from_raw(self.ptr as *mut c_char) })
            }
            ty if ty == TCStringType::CStr as u8 => {
                // SAFETY:
                //  - ptr was created by CStr::as_ptr
                //  - data was not modified since that time (caller promises)
                RustString::CStr(unsafe { CStr::from_ptr(self.ptr as *mut c_char) })
            }
            ty if ty == TCStringType::String as u8 => {
                // SAFETY:
                //  - ptr was created by string_into_raw_parts
                //  - data was not modified since that time (caller promises)
                RustString::String(unsafe {
                    String::from_raw_parts(self.ptr as *mut u8, self.len, self.cap)
                })
            }
            ty if ty == TCStringType::Bytes as u8 => {
                // SAFETY:
                //  - ptr was created by vec_into_raw_parts
                //  - data was not modified since that time (caller promises)
                RustString::Bytes(unsafe {
                    Vec::from_raw_parts(self.ptr as *mut u8, self.len, self.cap)
                })
            }
            _ => RustString::Null,
        }
    }

    fn as_ctype(arg: Self::RustType) -> Self {
        match arg {
            RustString::Null => Self {
                ty: TCStringType::Null as u8,
                ..Default::default()
            },
            RustString::CString(cstring) => Self {
                ty: TCStringType::CString as u8,
                ptr: cstring.into_raw() as *mut libc::c_void,
                ..Default::default()
            },
            RustString::CStr(cstr) => Self {
                ty: TCStringType::CStr as u8,
                ptr: cstr.as_ptr() as *mut libc::c_void,
                ..Default::default()
            },
            RustString::String(string) => {
                let (ptr, len, cap) = string_into_raw_parts(string);
                Self {
                    ty: TCStringType::String as u8,
                    ptr: ptr as *mut libc::c_void,
                    len,
                    cap,
                }
            }
            RustString::Bytes(bytes) => {
                let (ptr, len, cap) = vec_into_raw_parts(bytes);
                Self {
                    ty: TCStringType::Bytes as u8,
                    ptr: ptr as *mut libc::c_void,
                    len,
                    cap,
                }
            }
        }
    }
}

impl<'a> RustString<'a> {
    /// Get a regular Rust &str for this value.
    pub(crate) fn as_str(&mut self) -> Result<&str, std::str::Utf8Error> {
        match self {
            RustString::CString(cstring) => cstring.as_c_str().to_str(),
            RustString::CStr(cstr) => cstr.to_str(),
            RustString::String(ref string) => Ok(string.as_ref()),
            RustString::Bytes(_) => {
                self.bytes_to_string()?;
                self.as_str() // now the String variant, so won't recurse
            }
            RustString::Null => unreachable!(),
        }
    }

    /// Consume this RustString and return an equivalent String, or an error if not
    /// valid UTF-8.  In the error condition, the original data is lost.
    pub(crate) fn into_string(mut self) -> Result<String, std::str::Utf8Error> {
        match self {
            RustString::CString(cstring) => cstring.into_string().map_err(|e| e.utf8_error()),
            RustString::CStr(cstr) => cstr.to_str().map(|s| s.to_string()),
            RustString::String(string) => Ok(string),
            RustString::Bytes(_) => {
                self.bytes_to_string()?;
                self.into_string() // now the String variant, so won't recurse
            }
            RustString::Null => unreachable!(),
        }
    }

    pub(crate) fn as_bytes(&self) -> &[u8] {
        match self {
            RustString::CString(cstring) => cstring.as_bytes(),
            RustString::CStr(cstr) => cstr.to_bytes(),
            RustString::String(string) => string.as_bytes(),
            RustString::Bytes(bytes) => bytes.as_ref(),
            RustString::Null => unreachable!(),
        }
    }

    /// Convert the RustString, in place, from the Bytes to String variant.  On successful return,
    /// the RustString has variant RustString::String.
    fn bytes_to_string(&mut self) -> Result<(), std::str::Utf8Error> {
        let mut owned = RustString::Null;
        // temporarily swap a Null value into self; we'll swap that back
        // shortly.
        std::mem::swap(self, &mut owned);
        match owned {
            RustString::Bytes(bytes) => match String::from_utf8(bytes) {
                Ok(string) => {
                    *self = RustString::String(string);
                    Ok(())
                }
                Err(e) => {
                    let (e, bytes) = (e.utf8_error(), e.into_bytes());
                    // put self back as we found it
                    *self = RustString::Bytes(bytes);
                    Err(e)
                }
            },
            _ => {
                // not bytes, so just swap back
                std::mem::swap(self, &mut owned);
                Ok(())
            }
        }
    }

    /// Convert the RustString, in place, into one of the C variants.  If this is not
    /// possible, such as if the string contains an embedded NUL, then the string
    /// remains unchanged.
    fn string_to_cstring(&mut self) {
        let mut owned = RustString::Null;
        // temporarily swap a Null value into self; we'll swap that back shortly
        std::mem::swap(self, &mut owned);
        match owned {
            RustString::String(string) => {
                match CString::new(string) {
                    Ok(cstring) => {
                        *self = RustString::CString(cstring);
                    }
                    Err(nul_err) => {
                        // recover the underlying String from the NulError and restore
                        // the RustString
                        let original_bytes = nul_err.into_vec();
                        // SAFETY: original_bytes came from a String moments ago, so still valid utf8
                        let string = unsafe { String::from_utf8_unchecked(original_bytes) };
                        *self = RustString::String(string);
                    }
                }
            }
            _ => {
                // not a CString, so just swap back
                std::mem::swap(self, &mut owned);
            }
        }
    }

    pub(crate) fn to_path_buf_mut(&mut self) -> Result<PathBuf, std::str::Utf8Error> {
        #[cfg(unix)]
        let path: OsString = {
            // on UNIX, we can use the bytes directly, without requiring that they
            // be valid UTF-8.
            use std::ffi::OsStr;
            use std::os::unix::ffi::OsStrExt;
            OsStr::from_bytes(self.as_bytes()).to_os_string()
        };
        #[cfg(windows)]
        let path: OsString = {
            // on Windows, we assume the filename is valid Unicode, so it can be
            // represented as UTF-8.
            OsString::from(self.as_str()?.to_string())
        };
        Ok(path.into())
    }
}

impl<'a> From<String> for RustString<'a> {
    fn from(string: String) -> RustString<'a> {
        RustString::String(string)
    }
}

impl<'a> From<&str> for RustString<'static> {
    fn from(string: &str) -> RustString<'static> {
        RustString::String(string.to_string())
    }
}

/// Utility function to borrow a TCString from a pointer arg, modify it,
/// and restore it.
///
/// This implements a kind of "interior mutability", relying on the
/// single-threaded use of all TC* types.
///
/// # SAFETY
///
///  - tcstring must not be NULL
///  - *tcstring must be a valid TCString
///  - *tcstring must not be accessed by anything else, despite the *const
unsafe fn wrap<T, F>(tcstring: *const TCString, f: F) -> T
where
    F: FnOnce(&mut RustString) -> T,
{
    debug_assert!(!tcstring.is_null());

    // SAFETY:
    //  - we have exclusive to *tcstring (promised by caller)
    let tcstring = tcstring as *mut TCString;

    // SAFETY:
    //  - tcstring is not NULL
    //  - *tcstring is a valid string (promised by caller)
    let mut rstring = unsafe { TCString::take_val_from_arg(tcstring, TCString::default()) };

    let rv = f(&mut rstring);

    // update the caller's TCString with the updated RustString
    // SAFETY:
    //  - tcstring is not NULL (we just took from it)
    //  - tcstring points to valid memory (we just took from it)
    unsafe { TCString::val_to_arg_out(rstring, tcstring) };

    rv
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
    items: *mut TCString,
}

impl CList for TCStringList {
    type Element = TCString;

    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self {
        TCStringList {
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

/// Create a new TCString referencing the given C string.  The C string must remain valid and
/// unchanged until after the TCString is freed.  It's typically easiest to ensure this by using a
/// static string.
///
/// NOTE: this function does _not_ take responsibility for freeing the given C string.  The
/// given string can be freed once the TCString referencing it has been freed.
///
/// For example:
///
/// ```text
/// char *url = get_item_url(..); // dynamically allocate C string
/// tc_task_annotate(task, tc_string_borrow(url)); // TCString created, passed, and freed
/// free(url); // string is no longer referenced and can be freed
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_string_borrow(cstr: *const libc::c_char) -> TCString {
    debug_assert!(!cstr.is_null());
    // SAFETY:
    //  - cstr is not NULL (promised by caller, verified by assertion)
    //  - cstr's lifetime exceeds that of the TCString (promised by caller)
    //  - cstr contains a valid NUL terminator (promised by caller)
    //  - cstr's content will not change before it is destroyed (promised by caller)
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    // SAFETY:
    //  - caller promises to free this string
    unsafe { TCString::return_val(RustString::CStr(cstr)) }
}

/// Create a new TCString by cloning the content of the given C string.  The resulting TCString
/// is independent of the given string, which can be freed or overwritten immediately.
#[no_mangle]
pub unsafe extern "C" fn tc_string_clone(cstr: *const libc::c_char) -> TCString {
    debug_assert!(!cstr.is_null());
    // SAFETY:
    //  - cstr is not NULL (promised by caller, verified by assertion)
    //  - cstr's lifetime exceeds that of this function (by C convention)
    //  - cstr contains a valid NUL terminator (promised by caller)
    //  - cstr's content will not change before it is destroyed (by C convention)
    let cstr: &CStr = unsafe { CStr::from_ptr(cstr) };
    let cstring: CString = cstr.into();
    // SAFETY:
    //  - caller promises to free this string
    unsafe { TCString::return_val(RustString::CString(cstring)) }
}

/// Create a new TCString containing the given string with the given length. This allows creation
/// of strings containing embedded NUL characters.  As with `tc_string_clone`, the resulting
/// TCString is independent of the passed buffer, which may be reused or freed immediately.
///
/// The length should _not_ include any trailing NUL.
///
/// The given length must be less than half the maximum value of usize.
#[no_mangle]
pub unsafe extern "C" fn tc_string_clone_with_len(
    buf: *const libc::c_char,
    len: usize,
) -> TCString {
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

    // SAFETY:
    //  - caller promises to free this string
    unsafe { TCString::return_val(RustString::Bytes(vec)) }
}

/// Get the content of the string as a regular C string.  The given string must be valid.  The
/// returned value is NULL if the string contains NUL bytes or (in some cases) invalid UTF-8.  The
/// returned C string is valid until the TCString is freed or passed to another TC API function.
///
/// In general, prefer [`tc_string_content_with_len`] except when it's certain that the string is
/// valid and NUL-free.
///
/// This function takes the TCString by pointer because it may be modified in-place to add a NUL
/// terminator.  The pointer must not be NULL.
///
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub unsafe extern "C" fn tc_string_content(tcstring: *const TCString) -> *const libc::c_char {
    // SAFETY;
    //  - tcstring is not NULL (promised by caller)
    //  - *tcstring is valid (promised by caller)
    //  - *tcstring is not accessed concurrently (single-threaded)
    unsafe {
        wrap(tcstring, |rstring| {
            // try to eliminate the Bytes variant.  If this fails, we'll return NULL
            // below, so the error is ignorable.
            let _ = rstring.bytes_to_string();

            // and eliminate the String variant
            rstring.string_to_cstring();

            match &rstring {
                RustString::CString(cstring) => cstring.as_ptr(),
                RustString::String(_) => std::ptr::null(), // string_to_cstring failed
                RustString::CStr(cstr) => cstr.as_ptr(),
                RustString::Bytes(_) => std::ptr::null(), // already returned above
                RustString::Null => unreachable!(),
            }
        })
    }
}

/// Get the content of the string as a pointer and length.  The given string must not be NULL.
/// This function can return any string, even one including NUL bytes or invalid UTF-8.  The
/// returned buffer is valid until the TCString is freed or passed to another TaskChampio
/// function.
///
/// This function takes the TCString by pointer because it may be modified in-place to add a NUL
/// terminator.  The pointer must not be NULL.
///
/// This function does _not_ take ownership of the TCString.
#[no_mangle]
pub unsafe extern "C" fn tc_string_content_with_len(
    tcstring: *const TCString,
    len_out: *mut usize,
) -> *const libc::c_char {
    // SAFETY;
    //  - tcstring is not NULL (promised by caller)
    //  - *tcstring is valid (promised by caller)
    //  - *tcstring is not accessed concurrently (single-threaded)
    unsafe {
        wrap(tcstring, |rstring| {
            let bytes = rstring.as_bytes();

            // SAFETY:
            //  - len_out is not NULL (promised by caller)
            //  - len_out points to valid memory (promised by caller)
            //  - len_out is properly aligned (C convention)
            usize::val_to_arg_out(bytes.len(), len_out);
            bytes.as_ptr() as *const libc::c_char
        })
    }
}

/// Free a TCString.  The given string must not be NULL.  The string must not be used
/// after this function returns, and must not be freed more than once.
#[no_mangle]
pub unsafe extern "C" fn tc_string_free(tcstring: *mut TCString) {
    // SAFETY:
    //  - tcstring is not NULL (promised by caller)
    //  - caller is exclusive owner of tcstring (promised by caller)
    drop(unsafe { TCString::take_val_from_arg(tcstring, TCString::default()) });
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
    unsafe { drop_value_list(tcstrings) };
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn empty_list_has_non_null_pointer() {
        let tcstrings = unsafe { TCStringList::return_val(Vec::new()) };
        assert!(!tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcstrings = unsafe { TCStringList::return_val(Vec::new()) };
        // SAFETY: testing expected behavior
        unsafe { tc_string_list_free(&mut tcstrings) };
        assert!(tcstrings.items.is_null());
        assert_eq!(tcstrings.len, 0);
        assert_eq!(tcstrings._capacity, 0);
    }

    const INVALID_UTF8: &[u8] = b"abc\xf0\x28\x8c\x28";

    fn make_cstring() -> RustString<'static> {
        RustString::CString(CString::new("a string").unwrap())
    }

    fn make_cstr() -> RustString<'static> {
        let cstr = CStr::from_bytes_with_nul(b"a string\0").unwrap();
        RustString::CStr(&cstr)
    }

    fn make_string() -> RustString<'static> {
        RustString::String("a string".into())
    }

    fn make_string_with_nul() -> RustString<'static> {
        RustString::String("a \0 nul!".into())
    }

    fn make_invalid_bytes() -> RustString<'static> {
        RustString::Bytes(INVALID_UTF8.to_vec())
    }

    fn make_bytes() -> RustString<'static> {
        RustString::Bytes(b"bytes".to_vec())
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
    fn invalid_bytes_as_str() {
        let as_str_err = make_invalid_bytes().as_str().unwrap_err();
        assert_eq!(as_str_err.valid_up_to(), 3); // "abc" is valid
    }

    #[test]
    fn valid_bytes_as_str() {
        assert_eq!(make_bytes().as_str().unwrap(), "bytes");
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
    fn invalid_bytes_as_bytes() {
        assert_eq!(make_invalid_bytes().as_bytes(), INVALID_UTF8);
    }

    #[test]
    fn cstring_string_to_cstring() {
        let mut tcstring = make_cstring();
        tcstring.string_to_cstring();
        assert_eq!(tcstring, make_cstring()); // unchanged
    }

    #[test]
    fn cstr_string_to_cstring() {
        let mut tcstring = make_cstr();
        tcstring.string_to_cstring();
        assert_eq!(tcstring, make_cstr()); // unchanged
    }

    #[test]
    fn string_string_to_cstring() {
        let mut tcstring = make_string();
        tcstring.string_to_cstring();
        assert_eq!(tcstring, make_cstring()); // converted to CString, same content
    }

    #[test]
    fn string_with_nul_string_to_cstring() {
        let mut tcstring = make_string_with_nul();
        tcstring.string_to_cstring();
        assert_eq!(tcstring, make_string_with_nul()); // unchanged
    }

    #[test]
    fn bytes_string_to_cstring() {
        let mut tcstring = make_bytes();
        tcstring.string_to_cstring();
        assert_eq!(tcstring, make_bytes()); // unchanged
    }
}

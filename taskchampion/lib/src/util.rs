use crate::string::RustString;

pub(crate) fn err_to_ruststring(e: impl std::string::ToString) -> RustString<'static> {
    RustString::from(e.to_string())
}

/// An implementation of Vec::into_raw_parts, which is still unstable.  Returns ptr, len, cap.
pub(crate) fn vec_into_raw_parts<T>(vec: Vec<T>) -> (*mut T, usize, usize) {
    // emulate Vec::into_raw_parts():
    // - disable dropping the Vec with ManuallyDrop
    // - extract ptr, len, and capacity using those methods
    let mut vec = std::mem::ManuallyDrop::new(vec);
    (vec.as_mut_ptr(), vec.len(), vec.capacity())
}

/// An implementation of String::into_raw_parts, which is still unstable.  Returns ptr, len, cap.
pub(crate) fn string_into_raw_parts(string: String) -> (*mut u8, usize, usize) {
    // emulate String::into_raw_parts():
    // - disable dropping the String with ManuallyDrop
    // - extract ptr, len, and capacity using those methods
    let mut string = std::mem::ManuallyDrop::new(string);
    (string.as_mut_ptr(), string.len(), string.capacity())
}

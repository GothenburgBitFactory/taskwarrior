use crate::string::RustString;

pub(crate) fn err_to_ruststring(e: anyhow::Error) -> RustString<'static> {
    // The default `to_string` representation of `anyhow::Error` only shows the "outermost"
    // context, e.g., "Could not connect to server", and omits the juicy details about what
    // actually went wrong. So, join all of those contexts with `: ` for presentation to the C++
    // layer.
    let entire_msg = e
        .chain()
        .skip(1)
        .fold(e.to_string(), |a, b| format!("{}: {}", a, b));
    RustString::from(entire_msg)
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

/// A result from a TC operation.  Typically if this value is TC_RESULT_ERROR,
/// the associated object's `tc_.._error` method will return an error message.
/// cbindgen:prefix-with-name
/// cbindgen:rename-all=ScreamingSnakeCase
#[repr(i32)]
pub enum TCResult {
    Error = -1,
    Ok = 0,
}

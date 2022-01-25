/// A result combines a boolean success value with
/// an error response.  It is equivalent to `Result<bool, ()>`.
/// cbindgen:prefix-with-name
/// cbindgen:rename-all=ScreamingSnakeCase
#[repr(C)]
pub enum TCResult {
    True,
    False,
    Error,
}

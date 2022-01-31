// TODO: make true = 1, false = 0, error = -1
/// A result combines a boolean success value with
/// an error response.  It is equivalent to `Result<bool, ()>`.
/// cbindgen:prefix-with-name
/// cbindgen:rename-all=ScreamingSnakeCase
#[repr(C)]
pub enum TCResult {
    Error = -1,
    False = 0,
    True = 1,
}

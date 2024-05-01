#[ffizz_header::item]
#[ffizz(order = 100)]
/// ***** TCResult *****
///
/// A result from a TC operation.  Typically if this value is TC_RESULT_ERROR,
/// the associated object's `tc_.._error` method will return an error message.
///
/// ```c
/// enum TCResult
/// #ifdef __cplusplus
///   : int32_t
/// #endif // __cplusplus
/// {
///   TC_RESULT_ERROR = -1,
///   TC_RESULT_OK = 0,
/// };
/// #ifndef __cplusplus
/// typedef int32_t TCResult;
/// #endif // __cplusplus
/// ```
#[repr(i32)]
pub enum TCResult {
    Error = -1,
    Ok = 0,
}

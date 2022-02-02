use crate::string::TCString;
use std::ptr::NonNull;

// TODO: generalize to TCStrings?

/// TCTags represents a list of tags associated with a task.
///
/// The content of this struct must be treated as read-only.
///
/// The lifetime of a TCTags instance is independent of the task, and it
/// will remain valid even if the task is freed.
#[repr(C)]
pub struct TCTags {
    // TODO: can we use NonNull here?
    /// strings representing each tag. these remain owned by the
    /// TCTags instance and will be freed by tc_tags_free.
    tags: *const NonNull<TCString<'static>>,
    /// number of tags in tags
    num_tags: libc::size_t,
    /// total size of tags (internal use only)
    _capacity: libc::size_t,
}

impl Default for TCTags {
    fn default() -> Self {
        Self {
            tags: std::ptr::null_mut(),
            num_tags: 0,
            _capacity: 0,
        }
    }
}

impl TCTags {
    /// Create a Vec of TCStrings into a TCTags instance.
    pub(crate) fn new(tags: Vec<NonNull<TCString<'static>>>) -> Self {
        // emulate Vec::into_raw_parts():
        // - disable dropping the Vec with ManuallyDrop
        // - extract ptr, len, and capacity using those methods
        let mut tags = std::mem::ManuallyDrop::new(tags);
        Self {
            tags: tags.as_mut_ptr(),
            num_tags: tags.len(),
            _capacity: tags.capacity(),
        }
    }

    /// Convert a TCTags to a Vec<_>.
    ///
    /// # Safety
    ///
    /// Tags must be _exactly_ as created by [`new`]
    unsafe fn into_vec(self) -> Vec<NonNull<TCString<'static>>> {
        // SAFETY:
        //
        // * tags.tags needs to have been previously allocated via Vec<*mut TCString>
        // * TCString needs to have the same size and alignment as what ptr was allocated with.
        // * length needs to be less than or equal to capacity.
        // * capacity needs to be the capacity that the pointer was allocated with.
        // * vec elements are not NULL
        //
        // All of this is true for a value returned from `new`, which the caller promised
        // not to change.
        unsafe { Vec::from_raw_parts(self.tags as *mut _, self.num_tags, self._capacity) }
    }
}

/// Free a TCTags instance.  The given pointer must not be NULL.  The instance must not be used
/// after this call.
#[no_mangle]
pub extern "C" fn tc_tags_free<'a>(tctags: *mut TCTags) {
    debug_assert!(!tctags.is_null());
    // SAFETY:
    //  - tctags is not NULL
    //  - tctags is valid (caller promises it has not been changed)
    //  - caller will not use the TCTags after this (promised by caller)
    let tctags: &'a mut TCTags = unsafe { &mut *tctags };

    debug_assert!(!tctags.tags.is_null());

    // replace the caller's TCTags with one containing a NULL pointer
    let tctags: TCTags = std::mem::take(tctags);

    // convert to a regular Vec
    // SAFETY:
    //  - tctags is exactly as returned from TCTags::new (promised by caller)
    let mut vec: Vec<_> = unsafe { tctags.into_vec() };

    // drop each contained string
    for tcstring in vec.drain(..) {
        // SAFETY:
        //  - the pointer is not NULL (as created by TCString::new)
        //  - C does not expect the string's lifetime to exceed this function
        drop(unsafe { TCString::from_arg(tcstring.as_ptr()) });
    }

    drop(vec);
}

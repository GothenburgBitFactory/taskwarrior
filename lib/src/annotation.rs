use crate::traits::*;
use crate::types::*;
use chrono::prelude::*;

/// TCAnnotation contains the details of an annotation.
#[repr(C)]
pub struct TCAnnotation {
    /// Time the annotation was made.  Must be nonzero.
    pub entry: libc::time_t,
    /// Content of the annotation.  Must not be NULL.
    pub description: *mut TCString<'static>,
}

impl PassByValue for TCAnnotation {
    // NOTE: we cannot use `RustType = Annotation` here because conversion of the
    // TCString to a String can fail.
    type RustType = (DateTime<Utc>, TCString<'static>);

    unsafe fn from_ctype(self) -> Self::RustType {
        // SAFETY:
        //  - any time_t value is valid
        //  - time_t is not zero, so unwrap is safe (see type docstring)
        let entry = unsafe { self.entry.from_ctype() }.unwrap();
        // SAFETY:
        //  - self is owned, so we can take ownership of this TCString
        //  - self.description is a valid, non-null TCString (see type docstring)
        let description = unsafe { TCString::take_from_ptr_arg(self.description) };
        (entry, description)
    }

    fn as_ctype((entry, description): Self::RustType) -> Self {
        TCAnnotation {
            entry: libc::time_t::as_ctype(Some(entry)),
            // SAFETY: caller assumes ownership of this value
            description: unsafe { description.return_ptr() },
        }
    }
}

impl Default for TCAnnotation {
    fn default() -> Self {
        TCAnnotation {
            entry: 0 as libc::time_t,
            description: std::ptr::null_mut(),
        }
    }
}

/// TCAnnotationList represents a list of annotations.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCAnnotationList {
    /// number of annotations in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// array of annotations. these remain owned by the TCAnnotationList instance and will be freed by
    /// tc_annotation_list_free.  This pointer is never NULL for a valid TCAnnotationList.
    items: *const TCAnnotation,
}

impl CList for TCAnnotationList {
    type Element = TCAnnotation;

    unsafe fn from_raw_parts(items: *const Self::Element, len: usize, cap: usize) -> Self {
        TCAnnotationList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const Self::Element, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCAnnotation instance.  The instance, and the TCString it contains, must not be used
/// after this call.
#[no_mangle]
pub unsafe extern "C" fn tc_annotation_free(tcann: *mut TCAnnotation) {
    debug_assert!(!tcann.is_null());
    // SAFETY:
    //  - *tcann is a valid TCAnnotation (caller promises to treat it as read-only)
    let annotation = unsafe { TCAnnotation::take_val_from_arg(tcann, TCAnnotation::default()) };
    drop(annotation);
}

/// Free a TCAnnotationList instance.  The instance, and all TCAnnotations it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCAnnotationList.
#[no_mangle]
pub unsafe extern "C" fn tc_annotation_list_free(tcanns: *mut TCAnnotationList) {
    // SAFETY:
    //  - tcanns is not NULL and points to a valid TCAnnotationList (caller is not allowed to
    //    modify the list)
    //  - caller promises not to use the value after return
    unsafe { drop_value_list(tcanns) }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_list_has_non_null_pointer() {
        let tcanns = TCAnnotationList::return_val(Vec::new());
        assert!(!tcanns.items.is_null());
        assert_eq!(tcanns.len, 0);
        assert_eq!(tcanns._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcanns = TCAnnotationList::return_val(Vec::new());
        // SAFETY: testing expected behavior
        unsafe { tc_annotation_list_free(&mut tcanns) };
        assert!(tcanns.items.is_null());
        assert_eq!(tcanns.len, 0);
        assert_eq!(tcanns._capacity, 0);
    }
}

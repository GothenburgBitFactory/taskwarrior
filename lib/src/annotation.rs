use crate::traits::*;
use crate::types::*;
use chrono::prelude::*;
use taskchampion::Annotation;

/// TCAnnotation contains the details of an annotation.
#[repr(C)]
pub struct TCAnnotation {
    /// Time the annotation was made, as a UNIX epoch timestamp
    pub entry: i64,
    /// Content of the annotation
    pub description: *mut TCString<'static>,
}

impl PassByValue for TCAnnotation {
    type RustType = Annotation;

    unsafe fn from_ctype(self) -> Annotation {
        let entry = Utc.timestamp(self.entry, 0);
        // SAFETY:
        //  - self is owned, so we can take ownership of this TCString
        //  - self.description was created from a String so has valid UTF-8
        //  - caller did not change it (promised by caller)
        let description = unsafe { TCString::take_from_arg(self.description) }
            .into_string()
            .unwrap();
        Annotation { entry, description }
    }

    fn as_ctype(arg: Annotation) -> Self {
        let description: TCString = arg.description.into();
        TCAnnotation {
            entry: arg.entry.timestamp(),
            // SAFETY: caller will later free this value via tc_annotation_free
            description: unsafe { description.return_val() },
        }
    }
}

impl Default for TCAnnotation {
    fn default() -> Self {
        TCAnnotation {
            entry: 0,
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

impl CArray for TCAnnotationList {
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
    let annotation = unsafe { TCAnnotation::take_from_arg(tcann, TCAnnotation::default()) };
    drop(annotation);
}

/// Free a TCAnnotationList instance.  The instance, and all TCAnnotations it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCAnnotationList.
#[no_mangle]
pub unsafe extern "C" fn tc_annotation_list_free(tcanns: *mut TCAnnotationList) {
    debug_assert!(!tcanns.is_null());
    // SAFETY:
    //  - *tcanns is a valid TCAnnotationList (caller promises to treat it as read-only)
    let annotations =
        unsafe { TCAnnotationList::take_from_arg(tcanns, TCAnnotationList::null_value()) };
    TCAnnotationList::drop_vector(annotations);
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_array_has_non_null_pointer() {
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

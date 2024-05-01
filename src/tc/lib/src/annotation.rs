use crate::traits::*;
use crate::types::*;
use taskchampion::chrono::prelude::*;

#[ffizz_header::item]
#[ffizz(order = 400)]
/// ***** TCAnnotation *****
///
/// TCAnnotation contains the details of an annotation.
///
/// # Safety
///
/// An annotation must be initialized from a tc_.. function, and later freed
/// with `tc_annotation_free` or `tc_annotation_list_free`.
///
/// Any function taking a `*TCAnnotation` requires:
///  - the pointer must not be NUL;
///  - the pointer must be one previously returned from a tc_… function;
///  - the memory referenced by the pointer must never be modified by C code; and
///  - ownership transfers to the called function, and the value must not be used
///    after the call returns.  In fact, the value will be zeroed out to ensure this.
///
/// TCAnnotations are not threadsafe.
///
/// ```c
/// typedef struct TCAnnotation {
///   // Time the annotation was made.  Must be nonzero.
///   time_t entry;
///   // Content of the annotation.  Must not be NULL.
///   TCString description;
/// } TCAnnotation;
/// ```
#[repr(C)]
pub struct TCAnnotation {
    pub entry: libc::time_t,
    pub description: TCString,
}

impl PassByValue for TCAnnotation {
    // NOTE: we cannot use `RustType = Annotation` here because conversion of the
    // Rust to a String can fail.
    type RustType = (DateTime<Utc>, RustString<'static>);

    unsafe fn from_ctype(mut self) -> Self::RustType {
        // SAFETY:
        //  - any time_t value is valid
        //  - time_t is copy, so ownership is not important
        let entry = unsafe { libc::time_t::val_from_arg(self.entry) }.unwrap();
        // SAFETY:
        //  - self.description is valid (came from return_val in as_ctype)
        //  - self is owned, so we can take ownership of this TCString
        let description =
            unsafe { TCString::take_val_from_arg(&mut self.description, TCString::default()) };
        (entry, description)
    }

    fn as_ctype((entry, description): Self::RustType) -> Self {
        TCAnnotation {
            entry: libc::time_t::as_ctype(Some(entry)),
            // SAFETY:
            //  - ownership of the TCString tied to ownership of Self
            description: unsafe { TCString::return_val(description) },
        }
    }
}

impl Default for TCAnnotation {
    fn default() -> Self {
        TCAnnotation {
            entry: 0 as libc::time_t,
            description: TCString::default(),
        }
    }
}

#[ffizz_header::item]
#[ffizz(order = 410)]
/// ***** TCAnnotationList *****
///
/// TCAnnotationList represents a list of annotations.
///
/// The content of this struct must be treated as read-only.
///
/// ```c
/// typedef struct TCAnnotationList {
///   // number of annotations in items
///   size_t len;
///   // reserved
///   size_t _u1;
///   // Array of annotations. These remain owned by the TCAnnotationList instance and will be freed by
///   // tc_annotation_list_free.  This pointer is never NULL for a valid TCAnnotationList.
///   struct TCAnnotation *items;
/// } TCAnnotationList;
/// ```
#[repr(C)]
pub struct TCAnnotationList {
    len: libc::size_t,
    /// total size of items (internal use only)
    capacity: libc::size_t,
    items: *mut TCAnnotation,
}

impl CList for TCAnnotationList {
    type Element = TCAnnotation;

    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self {
        TCAnnotationList {
            len,
            capacity: cap,
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
        (self.items, self.len, self.capacity)
    }
}

#[ffizz_header::item]
#[ffizz(order = 401)]
/// Free a TCAnnotation instance.  The instance, and the TCString it contains, must not be used
/// after this call.
///
/// ```c
/// EXTERN_C void tc_annotation_free(struct TCAnnotation *tcann);
/// ```
#[no_mangle]
pub unsafe extern "C" fn tc_annotation_free(tcann: *mut TCAnnotation) {
    debug_assert!(!tcann.is_null());
    // SAFETY:
    //  - tcann is not NULL
    //  - *tcann is a valid TCAnnotation (caller promised to treat it as read-only)
    let annotation = unsafe { TCAnnotation::take_val_from_arg(tcann, TCAnnotation::default()) };
    drop(annotation);
}

#[ffizz_header::item]
#[ffizz(order = 411)]
/// Free a TCAnnotationList instance.  The instance, and all TCAnnotations it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCAnnotationList.
///
/// ```c
/// EXTERN_C void tc_annotation_list_free(struct TCAnnotationList *tcanns);
/// ```
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
        let tcanns = unsafe { TCAnnotationList::return_val(Vec::new()) };
        assert!(!tcanns.items.is_null());
        assert_eq!(tcanns.len, 0);
        assert_eq!(tcanns.capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcanns = unsafe { TCAnnotationList::return_val(Vec::new()) };
        // SAFETY: testing expected behavior
        unsafe { tc_annotation_list_free(&mut tcanns) };
        assert!(tcanns.items.is_null());
        assert_eq!(tcanns.len, 0);
        assert_eq!(tcanns.capacity, 0);
    }
}

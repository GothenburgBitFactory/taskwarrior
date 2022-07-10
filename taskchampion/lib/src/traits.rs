use crate::util::vec_into_raw_parts;
use std::ptr::NonNull;

/// Support for values passed to Rust by value.  These are represented as full structs in C.  Such
/// values are implicitly copyable, via C's struct assignment.
///
/// The Rust and C types may differ, with from_ctype and as_ctype converting between them.
/// Implement this trait for the C type.
///
/// The RustType must be droppable (not containing raw pointers).
pub(crate) trait PassByValue: Sized {
    type RustType;

    /// Convert a C value to a Rust value.
    ///
    /// # Safety
    ///
    /// `self` must be a valid CType.
    #[allow(clippy::wrong_self_convention)]
    unsafe fn from_ctype(self) -> Self::RustType;

    /// Convert a Rust value to a C value.
    fn as_ctype(arg: Self::RustType) -> Self;

    /// Take a value from C as an argument.
    ///
    /// # Safety
    ///
    /// - `self` must be a valid instance of the C type.  This is typically ensured either by
    ///   requiring that C code not modify it, or by defining the valid values in C comments.
    unsafe fn val_from_arg(arg: Self) -> Self::RustType {
        // SAFETY:
        //  - arg is a valid CType (promised by caller)
        unsafe { arg.from_ctype() }
    }

    /// Take a value from C as a pointer argument, replacing it with the given value.  This is used
    /// to invalidate the C value as an additional assurance against subsequent use of the value.
    ///
    /// # Safety
    ///
    /// - arg must not be NULL
    /// - *arg must be a valid, properly aligned instance of the C type
    unsafe fn take_val_from_arg(arg: *mut Self, mut replacement: Self) -> Self::RustType {
        // SAFETY:
        //  - arg is valid (promised by caller)
        //  - replacement is valid and aligned (guaranteed by Rust)
        unsafe { std::ptr::swap(arg, &mut replacement) };
        // SAFETY:
        //  - replacement (formerly *arg) is a valid CType (promised by caller)
        unsafe { PassByValue::val_from_arg(replacement) }
    }

    /// Return a value to C
    ///
    /// # Safety
    ///
    /// - if the value is allocated, the caller must ensure that the value is eventually freed
    unsafe fn return_val(arg: Self::RustType) -> Self {
        Self::as_ctype(arg)
    }

    /// Return a value to C, via an "output parameter"
    ///
    /// # Safety
    ///
    /// - `arg_out` must not be NULL and must be properly aligned and pointing to valid memory
    ///   of the size of CType.
    unsafe fn val_to_arg_out(val: Self::RustType, arg_out: *mut Self) {
        debug_assert!(!arg_out.is_null());
        // SAFETY:
        //  - arg_out is not NULL (promised by caller, asserted)
        //  - arg_out is properly aligned and points to valid memory (promised by caller)
        unsafe { *arg_out = Self::as_ctype(val) };
    }
}

/// Support for values passed to Rust by pointer.  These are represented as opaque structs in C,
/// and always handled as pointers.
pub(crate) trait PassByPointer: Sized {
    /// Take a value from C as an argument.
    ///
    /// # Safety
    ///
    /// - arg must not be NULL
    /// - arg must be a value returned from Box::into_raw (via return_ptr or ptr_to_arg_out)
    /// - arg becomes invalid and must not be used after this call
    unsafe fn take_from_ptr_arg(arg: *mut Self) -> Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see docstring
        unsafe { *(Box::from_raw(arg)) }
    }

    /// Borrow a value from C as an argument.
    ///
    /// # Safety
    ///
    /// - arg must not be NULL
    /// - *arg must be a valid instance of Self
    /// - arg must be valid for the lifetime assigned by the caller
    /// - arg must not be modified by anything else during that lifetime
    unsafe fn from_ptr_arg_ref<'a>(arg: *const Self) -> &'a Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see docstring
        unsafe { &*arg }
    }

    /// Mutably borrow a value from C as an argument.
    ///
    /// # Safety
    ///
    /// - arg must not be NULL
    /// - *arg must be a valid instance of Self
    /// - arg must be valid for the lifetime assigned by the caller
    /// - arg must not be accessed by anything else during that lifetime
    unsafe fn from_ptr_arg_ref_mut<'a>(arg: *mut Self) -> &'a mut Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see docstring
        unsafe { &mut *arg }
    }

    /// Return a value to C, transferring ownership
    ///
    /// # Safety
    ///
    /// - the caller must ensure that the value is eventually freed
    unsafe fn return_ptr(self) -> *mut Self {
        Box::into_raw(Box::new(self))
    }

    /// Return a value to C, transferring ownership, via an "output parameter".
    ///
    /// # Safety
    ///
    /// - the caller must ensure that the value is eventually freed
    /// - arg_out must not be NULL
    /// - arg_out must point to valid, properly aligned memory for a pointer value
    unsafe fn ptr_to_arg_out(self, arg_out: *mut *mut Self) {
        debug_assert!(!arg_out.is_null());
        // SAFETY: see docstring
        unsafe { *arg_out = self.return_ptr() };
    }
}

/// Support for C lists of objects referenced by value.
///
/// The underlying C type should have three fields, containing items, length, and capacity.  The
/// required trait functions just fetch and set these fields.
///
/// The PassByValue trait will be implemented automatically, converting between the C type and
/// `Vec<Element>`.
///
/// The element type can be PassByValue or PassByPointer.  If the latter, it should use either
/// `NonNull<T>` or `Option<NonNull<T>>` to represent the element.  The latter is an "optional
/// pointer list", where elements can be omitted.
///
/// For most cases, it is only necessary to implement `tc_.._free` that calls one of the
/// drop_..._list functions.
///
/// # Safety
///
/// The C type must be documented as read-only.  None of the fields may be modified, nor anything
/// accessible via the `items` array.  The exception is modification via "taking" elements.
///
/// This class guarantees that the items pointer is non-NULL for any valid list (even when len=0).
pub(crate) trait CList: Sized {
    type Element;

    /// Create a new CList from the given items, len, and capacity.
    ///
    /// # Safety
    ///
    /// The arguments must either:
    ///  - be NULL, 0, and 0, respectively; or
    ///  - be valid for Vec::from_raw_parts
    unsafe fn from_raw_parts(items: *mut Self::Element, len: usize, cap: usize) -> Self;

    /// Return a mutable slice representing the elements in this list.
    fn slice(&mut self) -> &mut [Self::Element];

    /// Get the items, len, and capacity (in that order) for this instance.  These must be
    /// precisely the same values passed tearlier to `from_raw_parts`.
    fn into_raw_parts(self) -> (*mut Self::Element, usize, usize);

    /// Generate a NULL value.  By default this is a NULL items pointer with zero length and
    /// capacity.
    fn null_value() -> Self {
        // SAFETY:
        //  - satisfies the first case in from_raw_parts' safety documentation
        unsafe { Self::from_raw_parts(std::ptr::null_mut(), 0, 0) }
    }
}

/// Given a CList containing pass-by-value values, drop all of the values and
/// the list.
///
/// This is a convenience function for `tc_.._list_free` functions.
///
/// # Safety
///
/// - List must be non-NULL and point to a valid CL instance
/// - The caller must not use the value array points to after this function, as
///   it has been freed.  It will be replaced with the null value.
pub(crate) unsafe fn drop_value_list<CL, T>(list: *mut CL)
where
    CL: CList<Element = T>,
    T: PassByValue,
{
    debug_assert!(!list.is_null());

    // SAFETY:
    //  - *list is a valid CL (promised by caller)
    let mut vec = unsafe { CL::take_val_from_arg(list, CL::null_value()) };

    // first, drop each of the elements in turn
    for e in vec.drain(..) {
        // SAFETY:
        //  - e is a valid Element (promised by caller)
        //  - e is owned
        drop(unsafe { PassByValue::val_from_arg(e) });
    }
    // then drop the vector
    drop(vec);
}

/// Given a CList containing NonNull pointers, drop all of the pointed-to values and the list.
///
/// This is a convenience function for `tc_.._list_free` functions.
///
/// # Safety
///
/// - List must be non-NULL and point to a valid CL instance
/// - The caller must not use the value array points to after this function, as
///   it has been freed.  It will be replaced with the null value.
#[allow(dead_code)] // this was useful once, and might be again?
pub(crate) unsafe fn drop_pointer_list<CL, T>(list: *mut CL)
where
    CL: CList<Element = NonNull<T>>,
    T: PassByPointer,
{
    debug_assert!(!list.is_null());
    // SAFETY:
    //  - *list is a valid CL (promised by caller)
    let mut vec = unsafe { CL::take_val_from_arg(list, CL::null_value()) };

    // first, drop each of the elements in turn
    for e in vec.drain(..) {
        // SAFETY:
        //  - e is a valid Element (promised by caller)
        //  - e is owned
        drop(unsafe { PassByPointer::take_from_ptr_arg(e.as_ptr()) });
    }
    // then drop the vector
    drop(vec);
}

/// Given a CList containing optional pointers, drop all of the non-null pointed-to values and the
/// list.
///
/// This is a convenience function for `tc_.._list_free` functions, for lists from which items
/// can be taken.
///
/// # Safety
///
/// - List must be non-NULL and point to a valid CL instance
/// - The caller must not use the value array points to after this function, as
///   it has been freed.  It will be replaced with the null value.
pub(crate) unsafe fn drop_optional_pointer_list<CL, T>(list: *mut CL)
where
    CL: CList<Element = Option<NonNull<T>>>,
    T: PassByPointer,
{
    debug_assert!(!list.is_null());
    // SAFETY:
    //  - *list is a valid CL (promised by caller)
    let mut vec = unsafe { CL::take_val_from_arg(list, CL::null_value()) };

    // first, drop each of the elements in turn
    for e in vec.drain(..) {
        if let Some(e) = e {
            // SAFETY:
            //  - e is a valid Element (promised by caller)
            //  - e is owned
            drop(unsafe { PassByPointer::take_from_ptr_arg(e.as_ptr()) });
        }
    }
    // then drop the vector
    drop(vec);
}

/// Take a value from an optional pointer list, returning the value and replacing its array
/// element with NULL.
///
/// This is a convenience function for `tc_.._list_take` functions, for lists from which items
/// can be taken.
///
/// The returned value will be None if the element has already been taken, or if the index is
/// out of bounds.
///
/// # Safety
///
/// - List must be non-NULL and point to a valid CL instance
pub(crate) unsafe fn take_optional_pointer_list_item<CL, T>(
    list: *mut CL,
    index: usize,
) -> Option<NonNull<T>>
where
    CL: CList<Element = Option<NonNull<T>>>,
    T: PassByPointer,
{
    debug_assert!(!list.is_null());

    // SAFETy:
    //  - list is properly aligned, dereferencable, and points to an initialized CL, since it is valid
    //  - the lifetime of the resulting reference is limited to this function, during which time
    //    nothing else refers to this memory.
    let slice = unsafe { list.as_mut() }.unwrap().slice();
    if let Some(elt_ref) = slice.get_mut(index) {
        let mut rv = None;
        if let Some(elt) = elt_ref.as_mut() {
            rv = Some(*elt);
            *elt_ref = None; // clear out the array element
        }
        rv
    } else {
        None // index out of bounds
    }
}

impl<A> PassByValue for A
where
    A: CList,
{
    type RustType = Vec<A::Element>;

    unsafe fn from_ctype(self) -> Self::RustType {
        let (items, len, cap) = self.into_raw_parts();
        debug_assert!(!items.is_null());
        // SAFETY:
        //  - CList::from_raw_parts requires that items, len, and cap be valid for
        //    Vec::from_raw_parts if not NULL, and they are not NULL (as promised by caller)
        //  - CList::into_raw_parts returns precisely the values passed to from_raw_parts.
        //  - those parts are passed to Vec::from_raw_parts here.
        unsafe { Vec::from_raw_parts(items as *mut _, len, cap) }
    }

    fn as_ctype(arg: Self::RustType) -> Self {
        let (items, len, cap) = vec_into_raw_parts(arg);
        // SAFETY:
        //  - satisfies the second case in from_raw_parts' safety documentation
        unsafe { Self::from_raw_parts(items, len, cap) }
    }
}

use crate::util::vec_into_raw_parts;
use std::ptr::NonNull;

/// Support for values passed to Rust by value.  These are represented as full structs in C.  Such
/// values are implicitly copyable, via C's struct assignment.
///
/// The Rust and C types may differ, with from_ctype and as_ctype converting between them.
/// Implement this trait for the C type.
pub(crate) trait PassByValue: Sized {
    type RustType;

    /// Convert a C value to a Rust value.
    ///
    /// # Safety
    ///
    /// `self` must be a valid CType.
    unsafe fn from_ctype(self) -> Self::RustType;

    /// Convert a Rust value to a C value.
    fn as_ctype(arg: Self::RustType) -> Self;

    /// Take a value from C as an argument.
    ///
    /// # Safety
    ///
    /// `self` must be a valid CType.  This is typically ensured either by requiring that C
    /// code not modify it, or by defining the valid values in C comments.
    unsafe fn from_arg(arg: Self) -> Self::RustType {
        // SAFETY:
        //  - arg is a valid CType (promised by caller)
        unsafe { arg.from_ctype() }
    }

    /// Take a value from C as a pointer argument, replacing it with the given value.  This is used
    /// to invalidate the C value as an additional assurance against subsequent use of the value.
    ///
    /// # Safety
    ///
    /// `*arg` must be a valid CType, as with [`from_arg`].
    unsafe fn take_from_arg(arg: *mut Self, mut replacement: Self) -> Self::RustType {
        // SAFETY:
        //  - arg is valid (promised by caller)
        //  - replacement is valid (guaranteed by Rust)
        unsafe { std::ptr::swap(arg, &mut replacement) };
        // SAFETY:
        //  - replacement (formerly *arg) is a valid CType (promised by caller)
        unsafe { PassByValue::from_arg(replacement) }
    }

    /// Return a value to C
    fn return_val(arg: Self::RustType) -> Self {
        Self::as_ctype(arg)
    }

    /// Return a value to C, via an "output parameter"
    ///
    /// # Safety
    ///
    /// `arg_out` must not be NULL and must be properly aligned and pointing to valid memory
    /// of the size of CType.
    unsafe fn to_arg_out(val: Self::RustType, arg_out: *mut Self) {
        debug_assert!(!arg_out.is_null());
        // SAFETY:
        //  - arg_out is not NULL (promised by caller, asserted)
        //  - arg_out is properly aligned and points to valid memory (promised by caller)
        unsafe { *arg_out = Self::as_ctype(val) };
    }
}

/// Support for values passed to Rust by pointer.  These are represented as opaque structs in C,
/// and always handled as pointers.
///
/// # Safety
///
/// The functions provided by this trait are used directly in C interface functions, and make the
/// following expectations of the C code:
///
///  - When passing a value to Rust (via the `…arg…` functions),
///    - the pointer must not be NULL;
///    - the pointer must be one previously returned from Rust; and
///    - the memory addressed by the pointer must never be modified by C code.
///  - For `from_arg_ref`, the value must not be modified during the call to the Rust function
///  - For `from_arg_ref_mut`, the value must not be accessed (read or write) during the call
///    (these last two points are trivially ensured by all TC… types being non-threadsafe)
///  - For `take_from_arg`, the pointer becomes invalid and must not be used in _any way_ after it
///    is passed to the Rust function.
///  - For `return_val` and `to_arg_out`, it is the C caller's responsibility to later free the value.
///  - For `to_arg_out`, `arg_out` must not be NULL and must be properly aligned and pointing to
///    valid memory.
///
/// These requirements should be expressed in the C documentation for the type implementing this
/// trait.
pub(crate) trait PassByPointer: Sized {
    /// Take a value from C as an argument.
    ///
    /// # Safety
    ///
    /// See trait documentation.
    unsafe fn take_from_arg(arg: *mut Self) -> Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see trait documentation
        unsafe { *(Box::from_raw(arg)) }
    }

    /// Borrow a value from C as an argument.
    ///
    /// # Safety
    ///
    /// See trait documentation.
    unsafe fn from_arg_ref<'a>(arg: *const Self) -> &'a Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see trait documentation
        unsafe { &*arg }
    }

    /// Mutably borrow a value from C as an argument.
    ///
    /// # Safety
    ///
    /// See trait documentation.
    unsafe fn from_arg_ref_mut<'a>(arg: *mut Self) -> &'a mut Self {
        debug_assert!(!arg.is_null());
        // SAFETY: see trait documentation
        unsafe { &mut *arg }
    }

    /// Return a value to C, transferring ownership
    ///
    /// # Safety
    ///
    /// See trait documentation.
    unsafe fn return_val(self) -> *mut Self {
        Box::into_raw(Box::new(self))
    }

    /// Return a value to C, transferring ownership, via an "output parameter".
    ///
    /// # Safety
    ///
    /// See trait documentation.
    unsafe fn to_arg_out(self, arg_out: *mut *mut Self) {
        // SAFETY: see trait documentation
        unsafe {
            *arg_out = self.return_val();
        }
    }
}

/// Support for arrays of objects referenced by pointer.
///
/// The underlying C type should have three fields, containing items, length, and capacity.  The
/// required trait functions just fetch and set these fields.  The PassByValue trait will be
/// implemented automatically, converting between the C type and `Vec<NonNull<Element>>`.  For most
/// cases, it is only necessary to implement `tc_.._free` that first calls
/// `PassByValue::take_from_arg(arg, PointerArray::null_value())` to take the existing value and
/// replace it with the null value; then `PointerArray::drop_pointer_vector(..)` to drop the
/// resulting vector and all of the objects it points to.
///
/// # Safety
///
/// The C type must be documented as read-only.  None of the fields may be modified, nor anything
/// in the `items` array.
///
/// This class guarantees that the items pointer is non-NULL for any valid array (even when len=0),
/// and that all pointers at indexes 0..len-1 are non-NULL.
pub(crate) trait PointerArray: Sized {
    type Element: 'static + PassByPointer;

    /// Create a new PointerArray from the given items, len, and capacity.
    ///
    /// # Safety
    ///
    /// The arguments must either:
    ///  - be NULL, 0, and 0, respectively; or
    ///  - be valid for Vec::from_raw_parts
    unsafe fn from_raw_parts(items: *const NonNull<Self::Element>, len: usize, cap: usize) -> Self;

    /// Get the items, len, and capacity (in that order) for this instance.  These must be
    /// precisely the same values passed tearlier to `from_raw_parts`.
    fn into_raw_parts(self) -> (*const NonNull<Self::Element>, usize, usize);

    /// Generate a NULL value.  By default this is a NULL items pointer with zero length and
    /// capacity.
    fn null_value() -> Self {
        // SAFETY:
        //  - satisfies the first case in from_raw_parts' safety documentation
        unsafe { Self::from_raw_parts(std::ptr::null(), 0, 0) }
    }

    /// Drop a vector of element pointers.  This is a convenience function for implementing
    /// tc_.._free functions.
    fn drop_pointer_vector(mut vec: Vec<NonNull<Self::Element>>) {
        // first, drop each of the elements in turn
        for p in vec.drain(..) {
            // SAFETY:
            //  - p is not NULL (NonNull)
            //  - p was generated by Rust (true for all arrays)
            //  - p was not modified (all arrays are immutable from C)
            //  - caller will not use this pointer again (promised by caller; p has been drain'd from
            //    the vector)
            drop(unsafe { PassByPointer::take_from_arg(p.as_ptr()) });
        }
        drop(vec);
    }
}

impl<A> PassByValue for A
where
    A: PointerArray,
{
    type RustType = Vec<NonNull<A::Element>>;

    unsafe fn from_ctype(self) -> Self::RustType {
        let (items, len, cap) = self.into_raw_parts();
        debug_assert!(!items.is_null());
        // SAFETY:
        //  - PointerArray::from_raw_parts requires that items, len, and cap be valid for
        //    Vec::from_raw_parts if not NULL, and they are not NULL (as promised by caller)
        //  - PointerArray::into_raw_parts returns precisely the values passed to from_raw_parts.
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

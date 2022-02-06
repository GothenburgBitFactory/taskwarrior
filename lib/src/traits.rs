/// Support for values passed to Rust by value.  These are represented as full structs in C.  Such
/// values are implicitly copyable, via C's struct assignment.
///
/// The Rust and C types may differ, with from_ctype and as_ctype converting between them.
pub(crate) trait PassByValue: Sized {
    type CType;

    /// Convert a C value to a Rust value.
    ///
    /// # Safety
    ///
    /// `arg` must be a valid CType.
    unsafe fn from_ctype(arg: Self::CType) -> Self;

    /// Convert a Rust value to a C value.
    fn as_ctype(self) -> Self::CType;

    /// Take a value from C as an argument.
    ///
    /// # Safety
    ///
    /// `arg` must be a valid CType.  This is typically ensured either by requiring that C
    /// code not modify it, or by defining the valid values in C comments.
    unsafe fn from_arg(arg: Self::CType) -> Self {
        // SAFETY:
        //  - arg is a valid CType (promised by caller)
        unsafe { Self::from_ctype(arg) }
    }

    /// Take a value from C as a pointer argument, replacing it with the given value.  This is used
    /// to invalidate the C value as an additional assurance against subsequent use of the value.
    ///
    /// # Safety
    ///
    /// `*arg` must be a valid CType, as with [`from_arg`].
    unsafe fn take_from_arg(arg: *mut Self::CType, mut replacement: Self::CType) -> Self {
        // SAFETY:
        //  - arg is valid (promised by caller)
        //  - replacement is valid (guaranteed by Rust)
        unsafe { std::ptr::swap(arg, &mut replacement) };
        // SAFETY:
        //  - replacement (formerly *arg) is a valid CType (promised by caller)
        unsafe { PassByValue::from_arg(replacement) }
    }

    /// Return a value to C
    fn return_val(self) -> Self::CType {
        self.as_ctype()
    }

    /// Return a value to C, via an "output parameter"
    ///
    /// # Safety
    ///
    /// `arg_out` must not be NULL and must be properly aligned and pointing to valid memory
    /// of the size of CType.
    unsafe fn to_arg_out(self, arg_out: *mut Self::CType) {
        debug_assert!(!arg_out.is_null());
        // SAFETY:
        //  - arg_out is not NULL (promised by caller, asserted)
        //  - arg_out is properly aligned and points to valid memory (promised by caller)
        unsafe { *arg_out = self.as_ctype() };
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

use crate::traits::*;
use crate::types::*;

/// TCUDA contains the details of a UDA.
#[repr(C)]
pub struct TCUDA {
    /// Namespace of the UDA.  For legacy UDAs, this is NULL.
    pub ns: *mut TCString<'static>,
    /// UDA key.  Must not be NULL.
    pub key: *mut TCString<'static>,
    /// Content of the UDA.  Must not be NULL.
    pub value: *mut TCString<'static>,
}

pub(crate) struct UDA {
    pub ns: Option<TCString<'static>>,
    pub key: TCString<'static>,
    pub value: TCString<'static>,
}

impl PassByValue for TCUDA {
    type RustType = UDA;

    unsafe fn from_ctype(self) -> Self::RustType {
        UDA {
            ns: if self.ns.is_null() {
                None
            } else {
                // SAFETY:
                //  - self is owned, so we can take ownership of this TCString
                //  - self.ns is a valid, non-null TCString (NULL just checked)
                Some(unsafe { TCString::take_from_arg(self.ns) })
            },
            // SAFETY:
            //  - self is owned, so we can take ownership of this TCString
            //  - self.key is a valid, non-null TCString (see type docstring)
            key: unsafe { TCString::take_from_arg(self.key) },
            // SAFETY:
            //  - self is owned, so we can take ownership of this TCString
            //  - self.value is a valid, non-null TCString (see type docstring)
            value: unsafe { TCString::take_from_arg(self.value) },
        }
    }

    fn as_ctype(uda: UDA) -> Self {
        TCUDA {
            // SAFETY: caller assumes ownership of this value
            ns: if let Some(ns) = uda.ns {
                unsafe { ns.return_val() }
            } else {
                std::ptr::null_mut()
            },
            // SAFETY: caller assumes ownership of this value
            key: unsafe { uda.key.return_val() },
            // SAFETY: caller assumes ownership of this value
            value: unsafe { uda.value.return_val() },
        }
    }
}

impl Default for TCUDA {
    fn default() -> Self {
        TCUDA {
            ns: std::ptr::null_mut(),
            key: std::ptr::null_mut(),
            value: std::ptr::null_mut(),
        }
    }
}

/// TCUDAList represents a list of UDAs.
///
/// The content of this struct must be treated as read-only.
#[repr(C)]
pub struct TCUDAList {
    /// number of UDAs in items
    len: libc::size_t,

    /// total size of items (internal use only)
    _capacity: libc::size_t,

    /// array of UDAs. These remain owned by the TCUDAList instance and will be freed by
    /// tc_uda_list_free.  This pointer is never NULL for a valid TCUDAList.
    items: *const TCUDA,
}

impl CList for TCUDAList {
    type Element = TCUDA;

    unsafe fn from_raw_parts(items: *const Self::Element, len: usize, cap: usize) -> Self {
        TCUDAList {
            len,
            _capacity: cap,
            items,
        }
    }

    fn into_raw_parts(self) -> (*const Self::Element, usize, usize) {
        (self.items, self.len, self._capacity)
    }
}

/// Free a TCUDA instance.  The instance, and the TCStrings it contains, must not be used
/// after this call.
#[no_mangle]
pub unsafe extern "C" fn tc_uda_free(tcuda: *mut TCUDA) {
    debug_assert!(!tcuda.is_null());
    // SAFETY:
    //  - *tcuda is a valid TCUDA (caller promises to treat it as read-only)
    let uda = unsafe { TCUDA::take_from_arg(tcuda, TCUDA::default()) };
    drop(uda);
}

/// Free a TCUDAList instance.  The instance, and all TCUDAs it contains, must not be used after
/// this call.
///
/// When this call returns, the `items` pointer will be NULL, signalling an invalid TCUDAList.
#[no_mangle]
pub unsafe extern "C" fn tc_uda_list_free(tcudas: *mut TCUDAList) {
    // SAFETY:
    //  - tcudas is not NULL and points to a valid TCUDAList (caller is not allowed to
    //    modify the list)
    //  - caller promises not to use the value after return
    unsafe { drop_value_list(tcudas) }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn empty_list_has_non_null_pointer() {
        let tcudas = TCUDAList::return_val(Vec::new());
        assert!(!tcudas.items.is_null());
        assert_eq!(tcudas.len, 0);
        assert_eq!(tcudas._capacity, 0);
    }

    #[test]
    fn free_sets_null_pointer() {
        let mut tcudas = TCUDAList::return_val(Vec::new());
        // SAFETY: testing expected behavior
        unsafe { tc_uda_list_free(&mut tcudas) };
        assert!(tcudas.items.is_null());
        assert_eq!(tcudas.len, 0);
        assert_eq!(tcudas._capacity, 0);
    }
}

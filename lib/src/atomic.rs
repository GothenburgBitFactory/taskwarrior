//! Trait implementations for a few atomic types

use crate::traits::*;

impl PassByValue for usize {
    type RustType = usize;

    unsafe fn from_ctype(self) -> usize {
        self
    }

    fn as_ctype(arg: usize) -> usize {
        arg
    }
}

//! Trait implementations for a few atomic types

use crate::traits::*;

impl PassByValue for usize {
    type CType = usize;

    unsafe fn from_ctype(arg: usize) -> usize {
        arg
    }

    fn as_ctype(self) -> usize {
        self
    }
}

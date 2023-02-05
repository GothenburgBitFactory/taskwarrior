//! Trait implementations for a few atomic types

use crate::traits::*;
use taskchampion::chrono::{DateTime, Utc};
use taskchampion::utc_timestamp;

impl PassByValue for usize {
    type RustType = usize;

    unsafe fn from_ctype(self) -> usize {
        self
    }

    fn as_ctype(arg: usize) -> usize {
        arg
    }
}

/// Convert an Option<DateTime<Utc>> to a libc::time_t, or zero if not set.
impl PassByValue for libc::time_t {
    type RustType = Option<DateTime<Utc>>;

    unsafe fn from_ctype(self) -> Option<DateTime<Utc>> {
        if self != 0 {
            return Some(utc_timestamp(self));
        }
        None
    }

    fn as_ctype(arg: Option<DateTime<Utc>>) -> libc::time_t {
        arg.map(|ts| ts.timestamp() as libc::time_t)
            .unwrap_or(0 as libc::time_t)
    }
}

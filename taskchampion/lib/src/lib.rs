#![warn(unsafe_op_in_unsafe_fn)]
#![allow(unused_unsafe)]
// Not working yet in stable - https://github.com/rust-lang/rust-clippy/issues/8020
// #![warn(clippy::undocumented_unsafe_blocks)]

// docstrings for extern "C" functions are reflected into C, and do not benefit
// from safety docs.
#![allow(clippy::missing_safety_doc)]
// deny some things that are typically warnings
#![deny(clippy::derivable_impls)]
#![deny(clippy::wrong_self_convention)]
#![deny(clippy::extra_unused_lifetimes)]
#![deny(clippy::unnecessary_to_owned)]

// ffizz_header orders:
//
// 000-099: header matter
// 100-199: TCResult
// 200-299: TCString / List
// 300-399: TCUuid / List
// 400-499: TCAnnotation / List
// 500-599: TCUda / List
// 600-699: TCKV / List
// 700-799: TCStatus
// 800-899: TCServer
// 900-999: TCReplica
// 1000-1099: TCTask / List
// 1100-1199: TCWorkingSet
// 10000-10099: footer

ffizz_header::snippet! {
#[ffizz(name="intro", order=0)]
/// TaskChampion
///
/// This file defines the C interface to libtaskchampion.  This is a thin wrapper around the Rust
/// `taskchampion` crate.  Refer to the documentation for that crate at
/// https://docs.rs/taskchampion/latest/taskchampion/ for API details.  The comments in this file
/// focus mostly on the low-level details of passing values to and from TaskChampion.
///
/// # Overview
///
/// This library defines four major types used to interact with the API, that map directly to Rust
/// types.
///
///  * TCReplica - see https://docs.rs/taskchampion/latest/taskchampion/struct.Replica.html
///  * TCTask - see https://docs.rs/taskchampion/latest/taskchampion/struct.Task.html
///  * TCServer - see https://docs.rs/taskchampion/latest/taskchampion/trait.Server.html
///  * TCWorkingSet - see https://docs.rs/taskchampion/latest/taskchampion/struct.WorkingSet.html
///
/// It also defines a few utility types:
///
///  * TCString - a wrapper around both C (NUL-terminated) and Rust (always utf-8) strings.
///  * TC…List - a list of objects represented as a C array
///  * see below for the remainder
///
/// # Safety
///
/// Each type contains specific instructions to ensure memory safety.  The general rules are as
/// follows.
///
/// No types in this library are threadsafe.  All values should be used in only one thread for their
/// entire lifetime.  It is safe to use unrelated values in different threads (for example,
/// different threads may use different TCReplica values concurrently).
///
/// ## Pass by Pointer
///
/// Several types such as TCReplica and TCString are "opaque" types and always handled as pointers
/// in C. The bytes these pointers address are private to the Rust implementation and must not be
/// accessed from C.
///
/// Pass-by-pointer values have exactly one owner, and that owner is responsible for freeing the
/// value (using a `tc_…_free` function), or transferring ownership elsewhere.  Except where
/// documented otherwise, when a value is passed to C, ownership passes to C as well.  When a value
/// is passed to Rust, ownership stays with the C code.  The exception is TCString, ownership of
/// which passes to Rust when it is used as a function argument.
///
/// The limited circumstances where one value must not outlive another, due to pointer references
/// between them, are documented below.
///
/// ## Pass by Value
///
/// Types such as TCUuid and TC…List are passed by value, and contain fields that are accessible
/// from C.  C code is free to access the content of these types in a _read_only_ fashion.
///
/// Pass-by-value values that contain pointers also have exactly one owner, responsible for freeing
/// the value or transferring ownership.  The tc_…_free functions for these types will replace the
/// pointers with NULL to guard against use-after-free errors.  The interior pointers in such values
/// should never be freed directly (for example, `tc_string_free(tcuda.value)` is an error).
///
/// TCUuid is a special case, because it does not contain pointers.  It can be freely copied and
/// need not be freed.
///
/// ## Lists
///
/// Lists are a special kind of pass-by-value type.  Each contains `len` and `items`, where `items`
/// is an array of length `len`.  Lists, and the values in the `items` array, must be treated as
/// read-only.  On return from an API function, a list's ownership is with the C caller, which must
/// eventually free the list.  List data must be freed with the `tc_…_list_free` function.  It is an
/// error to free any value in the `items` array of a list.
}

ffizz_header::snippet! {
#[ffizz(name="topmatter", order=1)]
/// ```c
/// #ifndef TASKCHAMPION_H
/// #define TASKCHAMPION_H
///
/// #include <stdbool.h>
/// #include <stdint.h>
/// #include <time.h>
///
/// #ifdef __cplusplus
/// #define EXTERN_C extern "C"
/// #else
/// #define EXTERN_C
/// #endif // __cplusplus
/// ```
}

ffizz_header::snippet! {
#[ffizz(name="bottomatter", order=10000)]
/// ```c
/// #endif /* TASKCHAMPION_H */
/// ```
}

mod traits;
mod util;

pub mod annotation;
pub use annotation::*;
pub mod atomic;
pub mod kv;
pub use kv::*;
pub mod replica;
pub use replica::*;
pub mod result;
pub use result::*;
pub mod server;
pub use server::*;
pub mod status;
pub use status::*;
pub mod string;
pub use string::*;
pub mod task;
pub use task::*;
pub mod uda;
pub use uda::*;
pub mod uuid;
pub use uuid::*;
pub mod workingset;
pub use workingset::*;

pub(crate) mod types {
    pub(crate) use crate::annotation::{TCAnnotation, TCAnnotationList};
    pub(crate) use crate::kv::TCKVList;
    pub(crate) use crate::replica::TCReplica;
    pub(crate) use crate::result::TCResult;
    pub(crate) use crate::server::TCServer;
    pub(crate) use crate::status::TCStatus;
    pub(crate) use crate::string::{RustString, TCString, TCStringList};
    pub(crate) use crate::task::{TCTask, TCTaskList};
    pub(crate) use crate::uda::{TCUda, TCUdaList, Uda};
    pub(crate) use crate::uuid::{TCUuid, TCUuidList};
    pub(crate) use crate::workingset::TCWorkingSet;
}

#[cfg(debug_assertions)]
/// Generate the taskchapion.h header
pub fn generate_header() -> String {
    ffizz_header::generate()
}

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

mod traits;
mod util;

pub mod annotation;
pub use annotation::*;
pub mod atomic;
pub use atomic::*;
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
    pub(crate) use crate::kv::{TCKVList, TCKV};
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

#![warn(unsafe_op_in_unsafe_fn)]
// Not working yet in stable - https://github.com/rust-lang/rust-clippy/issues/8020
// #![warn(clippy::undocumented_unsafe_blocks)]

mod traits;
mod util;

pub mod annotation;
pub mod atomic;
pub mod replica;
pub mod result;
pub mod status;
pub mod string;
pub mod task;
pub mod uda;
pub mod uuid;

pub(crate) mod types {
    pub(crate) use crate::annotation::{TCAnnotation, TCAnnotationList};
    pub(crate) use crate::replica::TCReplica;
    pub(crate) use crate::result::TCResult;
    pub(crate) use crate::status::TCStatus;
    pub(crate) use crate::string::{TCString, TCStringList};
    pub(crate) use crate::task::{TCTask, TCTaskList};
    pub(crate) use crate::uda::{TCUDAList, TCUDA, UDA};
    pub(crate) use crate::uuid::{TCUuid, TCUuidList};
}

#![warn(unsafe_op_in_unsafe_fn)]
// Not working yet in stable - https://github.com/rust-lang/rust-clippy/issues/8020
// #![warn(clippy::undocumented_unsafe_blocks)]

mod traits;
mod util;

pub mod atomic;
pub mod replica;
pub mod result;
pub mod status;
pub mod string;
pub mod stringlist;
pub mod task;
pub mod tasklist;
pub mod uuid;
pub mod uuidlist;

pub(crate) mod types {
    pub(crate) use crate::replica::TCReplica;
    pub(crate) use crate::result::TCResult;
    pub(crate) use crate::status::TCStatus;
    pub(crate) use crate::string::TCString;
    pub(crate) use crate::stringlist::TCStringList;
    pub(crate) use crate::task::TCTask;
    pub(crate) use crate::tasklist::TCTaskList;
    pub(crate) use crate::uuid::TCUuid;
    pub(crate) use crate::uuidlist::TCUuidList;
}

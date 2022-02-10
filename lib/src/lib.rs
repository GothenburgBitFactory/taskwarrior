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

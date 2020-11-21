// TODO: remove this eventually when there's an API
#![allow(dead_code)]
#![allow(unused_variables)]

#[macro_use]
extern crate failure;

mod errors;
mod operation;
mod replica;
mod server;
mod task;
mod taskdb;
pub mod taskstorage;
mod util;

pub use operation::Operation;
pub use replica::Replica;
pub use server::Server;
pub use task::Priority;
pub use task::Status;
pub use task::Task;
pub use taskdb::DB;

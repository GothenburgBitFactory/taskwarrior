#![recursion_limit = "1024"]

extern crate chrono;
extern crate uuid;
#[macro_use]
extern crate error_chain;

mod tdb2;
mod task;
mod errors;

use std::io::BufRead;
pub use task::*;
pub use errors::*;

pub fn parse(filename: &str, reader: impl BufRead) -> Result<Vec<Task>> {
    Ok(tdb2::parse(filename, reader)?)
}

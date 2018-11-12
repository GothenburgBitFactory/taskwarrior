//! TDB2 is Taskwarrior's on-disk database format.  This module implements
//! support for the data structure as a  compatibility layer.

mod pig;
mod ff4;
pub(super) mod errors;

use std::io::BufRead;
use task::Task;
use self::ff4::parse_ff4;
use self::errors::*;

pub(crate) fn parse(filename: &str, reader: impl BufRead) -> Result<Vec<Task>> {
    let mut tasks = vec![];
    for (i, line) in reader.lines().enumerate() {
        tasks.push(parse_ff4(&line?).chain_err(|| {
            ErrorKind::ParseError(filename.to_string(), i as u64 + 1)
        })?);
    }
    Ok(tasks)
}

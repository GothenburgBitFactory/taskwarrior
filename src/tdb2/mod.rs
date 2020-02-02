//! TDB2 is Taskwarrior's on-disk database format.  This module implements
//! support for the data structure as a  compatibility layer.

mod ff4;

use self::ff4::parse_ff4;
use crate::task::Task;
use failure::Fallible;
use std::io::BufRead;

pub(crate) fn parse(filename: &str, reader: impl BufRead) -> Fallible<Vec<Task>> {
    let mut tasks = vec![];
    for (i, line) in reader.lines().enumerate() {
        tasks.push(parse_ff4(&line?).map_err(|e| {
            format_err!(
                "TDB2 Error at {}:{}: {}",
                filename.to_string(),
                i as u64 + 1,
                e
            )
        })?);
    }
    Ok(tasks)
}

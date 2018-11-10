mod nibbler;
mod ff4;

use std::io::{BufRead, Result};
use super::task::Task;
use self::ff4::parse_ff4;

pub(super) fn parse(reader: impl BufRead) -> Result<Vec<Task>> {
    let mut tasks = vec![];
    for line in reader.lines() {
        tasks.push(parse_ff4(&line?)?);
    }
    Ok(tasks)
}

mod pig;
mod ff4;

use std::io::BufRead;
use task::Task;
use self::ff4::parse_ff4;
use errors::*;

pub(super) fn parse(reader: impl BufRead) -> Result<Vec<Task>> {
    let mut tasks = vec![];
    for line in reader.lines() {
        tasks.push(parse_ff4(&line?)?);
    }
    Ok(tasks)
}

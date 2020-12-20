use crate::usage::Usage;
use failure::Fallible;
use std::io;

pub(crate) fn execute(command_name: String, summary: bool) -> Fallible<()> {
    let usage = Usage::new();
    usage.write_help(io::stdout(), command_name, summary)?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_summary() {
        execute("task".to_owned(), true).unwrap();
    }

    #[test]
    fn test_long() {
        execute("task".to_owned(), false).unwrap();
    }
}

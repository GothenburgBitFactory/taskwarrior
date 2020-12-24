use crate::usage::Usage;
use failure::Fallible;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    command_name: String,
    summary: bool,
) -> Fallible<()> {
    let usage = Usage::new();
    usage.write_help(w, command_name.as_ref(), summary)?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;

    #[test]
    fn test_summary() {
        let mut w = test_writer();
        execute(&mut w, s!("task"), true).unwrap();
    }

    #[test]
    fn test_long() {
        let mut w = test_writer();
        execute(&mut w, s!("task"), false).unwrap();
    }
}

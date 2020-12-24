use failure::Fallible;
use termcolor::{ColorSpec, WriteColor};

pub(crate) fn execute<W: WriteColor>(w: &mut W) -> Fallible<()> {
    write!(w, "TaskChampion ")?;
    w.set_color(ColorSpec::new().set_bold(true))?;
    writeln!(w, "{}", env!("CARGO_PKG_VERSION"))?;
    w.reset()?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;

    #[test]
    fn test_version() {
        let mut w = test_writer();
        execute(&mut w).unwrap();
        assert!(w.into_string().starts_with("TaskChampion "));
    }
}

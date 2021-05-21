use termcolor::{ColorSpec, WriteColor};

pub(crate) fn execute<W: WriteColor>(w: &mut W) -> Result<(), crate::Error> {
    write!(w, "TaskChampion ")?;
    w.set_color(ColorSpec::new().set_bold(true))?;
    write!(w, "{}", env!("CARGO_PKG_VERSION"))?;
    w.reset()?;

    if let Some(h) = option_env!("TC_GIT_REV") {
        write!(w, " (git rev: {})", h)?;
    }
    writeln!(w)?;
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

use crate::built_info;
use termcolor::{ColorSpec, WriteColor};

pub(crate) fn execute<W: WriteColor>(w: &mut W) -> Result<(), crate::Error> {
    write!(w, "TaskChampion ")?;
    w.set_color(ColorSpec::new().set_bold(true))?;
    write!(w, "{}", built_info::PKG_VERSION)?;
    w.reset()?;

    if let (Some(version), Some(dirty)) = (built_info::GIT_VERSION, built_info::GIT_DIRTY) {
        if dirty {
            write!(w, " (git version: {} with un-committed changes)", version)?;
        } else {
            write!(w, " (git version: {})", version)?;
        };
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

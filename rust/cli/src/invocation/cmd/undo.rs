use taskchampion::Replica;
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(w: &mut W, replica: &mut Replica) -> Result<(), crate::Error> {
    if replica.undo()? {
        writeln!(w, "Undo successful.")?;
    } else {
        writeln!(w, "Nothing to undo.")?;
    }
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_undo() {
        let mut w = test_writer();
        let mut replica = test_replica();

        // Note that the details of the actual undo operation are tested thoroughly in the taskchampion crate
        execute(&mut w, &mut replica).unwrap();
        assert_eq!(&w.into_string(), "Nothing to undo.\n")
    }
}

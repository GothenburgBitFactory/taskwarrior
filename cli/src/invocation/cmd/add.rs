use crate::argparse::{DescriptionMod, Modification};
use failure::Fallible;
use taskchampion::{Replica, Status};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    modification: Modification,
) -> Fallible<()> {
    let description = match modification.description {
        DescriptionMod::Set(ref s) => s.clone(),
        _ => "(no description)".to_owned(),
    };
    let t = replica.new_task(Status::Pending, description).unwrap();
    writeln!(w, "added task {}", t.get_uuid())?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;

    #[test]
    fn test_add() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let modification = Modification {
            description: DescriptionMod::Set("my description".to_owned()),
            ..Default::default()
        };
        execute(&mut w, &mut replica, modification).unwrap();

        // check that the task appeared..
        let task = replica.get_working_set_task(1).unwrap().unwrap();
        assert_eq!(task.get_description(), "my description");
        assert_eq!(task.get_status(), Status::Pending);

        assert_eq!(w.into_string(), format!("added task {}\n", task.get_uuid()));
    }
}

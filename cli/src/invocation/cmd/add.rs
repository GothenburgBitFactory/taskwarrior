use crate::argparse::{DescriptionMod, Modification};
use crate::invocation::apply_modification;
use taskchampion::{Replica, Status};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    mut modification: Modification,
) -> Result<(), crate::Error> {
    // extract the description from the modification to handle it specially
    let description = match modification.description {
        DescriptionMod::Set(ref s) => s.clone(),
        _ => "(no description)".to_owned(),
    };
    modification.description = DescriptionMod::None;

    let task = replica.new_task(Status::Pending, description).unwrap();
    let mut task = task.into_mut(replica);
    apply_modification(&mut task, &modification)?;
    writeln!(w, "added task {}", task.get_uuid())?;
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_add() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let modification = Modification {
            description: DescriptionMod::Set(s!("my description")),
            ..Default::default()
        };
        execute(&mut w, &mut replica, modification).unwrap();

        // check that the task appeared..
        let working_set = replica.working_set().unwrap();
        let task = replica
            .get_task(working_set.by_index(1).unwrap())
            .unwrap()
            .unwrap();
        assert_eq!(task.get_description(), "my description");
        assert_eq!(task.get_status(), Status::Pending);

        assert_eq!(w.into_string(), format!("added task {}\n", task.get_uuid()));
    }

    #[test]
    fn test_add_with_tags() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let modification = Modification {
            description: DescriptionMod::Set(s!("my description")),
            add_tags: vec![tag!("tag1")].drain(..).collect(),
            ..Default::default()
        };
        execute(&mut w, &mut replica, modification).unwrap();

        // check that the task appeared..
        let working_set = replica.working_set().unwrap();
        let task = replica
            .get_task(working_set.by_index(1).unwrap())
            .unwrap()
            .unwrap();
        assert_eq!(task.get_description(), "my description");
        assert_eq!(task.get_status(), Status::Pending);
        assert!(task.has_tag(&tag!("tag1")));

        assert_eq!(w.into_string(), format!("added task {}\n", task.get_uuid()));
    }
}

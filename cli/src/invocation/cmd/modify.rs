use crate::argparse::{Filter, Modification};
use crate::invocation::{apply_modification, filtered_tasks};
use failure::Fallible;
use taskchampion::Replica;

pub(crate) fn execute(
    replica: &mut Replica,
    filter: Filter,
    modification: Modification,
) -> Fallible<()> {
    for task in filtered_tasks(replica, &filter)? {
        let mut task = task.into_mut(replica);

        apply_modification(&mut task, &modification)?;
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::DescriptionMod;
    use crate::invocation::cmd::test::test_replica;
    use taskchampion::Status;

    #[test]
    fn test_modify() {
        let mut replica = test_replica();

        let task = replica
            .new_task(Status::Pending, "old description".to_owned())
            .unwrap();

        let filter = Filter {
            ..Default::default()
        };
        let modification = Modification {
            description: DescriptionMod::Set("new description".to_owned()),
            ..Default::default()
        };
        execute(&mut replica, filter, modification).unwrap();

        // check that the task appeared..
        let task = replica.get_task(task.get_uuid()).unwrap().unwrap();
        assert_eq!(task.get_description(), "new description");
        assert_eq!(task.get_status(), Status::Pending);
    }
}

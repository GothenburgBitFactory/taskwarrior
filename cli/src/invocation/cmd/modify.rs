use crate::argparse::Filter;
use crate::invocation::util::{confirm, summarize_task};
use crate::invocation::{apply_modification, filtered_tasks, ResolvedModification};
use crate::settings::Settings;
use taskchampion::Replica;
use termcolor::WriteColor;

/// confirm modification of more than `modificationt_count_prompt` tasks, defaulting to 3
fn check_modification<W: WriteColor>(
    w: &mut W,
    settings: &Settings,
    affected_tasks: usize,
) -> Result<bool, crate::Error> {
    let setting = settings.modification_count_prompt.unwrap_or(3);
    if setting == 0 || affected_tasks <= setting as usize {
        return Ok(true);
    }

    let prompt = format!("Operation will modify {} tasks; continue?", affected_tasks,);
    if confirm(&prompt)? {
        return Ok(true);
    }

    writeln!(w, "Cancelled")?;

    // only show this help if the setting is not set
    if settings.modification_count_prompt.is_none() {
        writeln!(
            w,
            "Set the `modification_count_prompt` setting to avoid this prompt:"
        )?;
        writeln!(
            w,
            "    ta config set modification_count_prompt {}",
            affected_tasks + 1
        )?;
        writeln!(w, "Set it to 0 to disable the prompt entirely")?;
    }
    Ok(false)
}

pub(in crate::invocation) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    settings: &Settings,
    filter: Filter,
    modification: ResolvedModification,
) -> Result<(), crate::Error> {
    let tasks = filtered_tasks(replica, &filter)?;

    if !check_modification(w, settings, tasks.size_hint().0)? {
        return Ok(());
    }

    for task in tasks {
        let mut task = task.into_mut(replica);

        apply_modification(&mut task, &modification)?;

        let task = task.into_immut();
        let summary = summarize_task(replica, &task)?;
        writeln!(w, "modified task {}", summary)?;
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::{DescriptionMod, Modification};
    use crate::invocation::test::test_replica;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;
    use taskchampion::Status;

    #[test]
    fn test_modify() {
        let mut w = test_writer();
        let mut replica = test_replica();
        let settings = Settings::default();

        let task = replica
            .new_task(Status::Pending, s!("old description"))
            .unwrap();

        let filter = Filter {
            ..Default::default()
        };
        let modification = ResolvedModification(Modification {
            description: DescriptionMod::Set(s!("new description")),
            ..Default::default()
        });
        execute(&mut w, &mut replica, &settings, filter, modification).unwrap();

        // check that the task appeared..
        let task = replica.get_task(task.get_uuid()).unwrap().unwrap();
        assert_eq!(task.get_description(), "new description");
        assert_eq!(task.get_status(), Status::Pending);

        assert_eq!(
            w.into_string(),
            format!("modified task 1 - new description\n")
        );
    }
}

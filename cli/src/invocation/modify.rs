use crate::argparse::{DescriptionMod, Modification};
use taskchampion::TaskMut;

/// Apply the given modification
pub(super) fn apply_modification(
    task: &mut TaskMut,
    modification: &Modification,
) -> anyhow::Result<()> {
    match modification.description {
        DescriptionMod::Set(ref description) => task.set_description(description.clone())?,
        DescriptionMod::Prepend(ref description) => {
            task.set_description(format!("{} {}", description, task.get_description()))?
        }
        DescriptionMod::Append(ref description) => {
            task.set_description(format!("{} {}", task.get_description(), description))?
        }
        DescriptionMod::None => {}
    }

    if let Some(ref status) = modification.status {
        task.set_status(status.clone())?;
    }

    if let Some(true) = modification.active {
        task.start()?;
    }

    if let Some(false) = modification.active {
        task.stop()?;
    }

    for tag in modification.add_tags.iter() {
        task.add_tag(&tag)?;
    }

    for tag in modification.remove_tags.iter() {
        task.remove_tag(&tag)?;
    }

    if let Some(wait) = modification.wait {
        task.set_wait(wait)?;
    }

    Ok(())
}

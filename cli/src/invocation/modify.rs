use crate::argparse::{DescriptionMod, Modification};
use failure::Fallible;
use taskchampion::TaskMut;
use termcolor::WriteColor;

/// Apply the given modification
pub(super) fn apply_modification<W: WriteColor>(
    w: &mut W,
    task: &mut TaskMut,
    modification: &Modification,
) -> Fallible<()> {
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

    write!(w, "modified task {}\n", task.get_uuid())?;

    Ok(())
}

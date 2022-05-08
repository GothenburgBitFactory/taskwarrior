use dialoguer::Confirm;
use taskchampion::{Replica, Task};

/// Print the prompt and ask the user to answer yes or no.  If input is not from a terminal, the
/// answer is assumed to be true.
pub(super) fn confirm<S: Into<String>>(prompt: S) -> anyhow::Result<bool> {
    if !atty::is(atty::Stream::Stdin) {
        return Ok(true);
    }
    Ok(Confirm::new().with_prompt(prompt).interact()?)
}

/// Summarize a task in a single line
pub(super) fn summarize_task(replica: &mut Replica, task: &Task) -> anyhow::Result<String> {
    let ws = replica.working_set()?;
    let uuid = task.get_uuid();
    if let Some(id) = ws.by_uuid(uuid) {
        Ok(format!("{} - {}", id, task.get_description()))
    } else {
        Ok(format!("{} - {}", uuid, task.get_description()))
    }
}

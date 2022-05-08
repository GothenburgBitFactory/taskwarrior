use crate::argparse::{DescriptionMod, Modification, TaskId};
use std::collections::HashSet;
use taskchampion::chrono::Utc;
use taskchampion::{Annotation, Replica, TaskMut};

/// A wrapper for Modification, promising that all TaskId instances are of variant TaskId::Uuid.
pub(super) struct ResolvedModification(pub(super) Modification);

/// Resolve a Modification to a ResolvedModification, based on access to a Replica.
///
/// This is not automatically done in `apply_modification` because, by that time, the TaskMut being
/// modified has an exclusive reference to the Replica, so it is impossible to search for matching
/// tasks.
pub(super) fn resolve_modification(
    unres: Modification,
    replica: &mut Replica,
) -> anyhow::Result<ResolvedModification> {
    Ok(ResolvedModification(Modification {
        description: unres.description,
        status: unres.status,
        wait: unres.wait,
        active: unres.active,
        add_tags: unres.add_tags,
        remove_tags: unres.remove_tags,
        add_dependencies: resolve_task_ids(replica, unres.add_dependencies)?,
        remove_dependencies: resolve_task_ids(replica, unres.remove_dependencies)?,
        annotate: unres.annotate,
    }))
}

/// Convert a set of arbitrary TaskId's into TaskIds containing only TaskId::Uuid.
fn resolve_task_ids(
    replica: &mut Replica,
    task_ids: HashSet<TaskId>,
) -> anyhow::Result<HashSet<TaskId>> {
    // already all UUIDs (or empty)?
    if task_ids.iter().all(|tid| matches!(tid, TaskId::Uuid(_))) {
        return Ok(task_ids);
    }

    let mut result = HashSet::new();
    let mut working_set = None;
    let mut all_tasks = None;
    for tid in task_ids {
        match tid {
            TaskId::WorkingSetId(i) => {
                let ws = match working_set {
                    Some(ref ws) => ws,
                    None => {
                        working_set = Some(replica.working_set()?);
                        working_set.as_ref().unwrap()
                    }
                };
                if let Some(u) = ws.by_index(i) {
                    result.insert(TaskId::Uuid(u));
                }
            }
            TaskId::PartialUuid(partial) => {
                let ts = match all_tasks {
                    Some(ref ts) => ts,
                    None => {
                        all_tasks = Some(
                            replica
                                .all_task_uuids()?
                                .drain(..)
                                .map(|u| (u, u.to_string()))
                                .collect::<Vec<_>>(),
                        );
                        all_tasks.as_ref().unwrap()
                    }
                };
                for (u, ustr) in ts {
                    if ustr.starts_with(&partial) {
                        result.insert(TaskId::Uuid(*u));
                    }
                }
            }
            TaskId::Uuid(u) => {
                result.insert(TaskId::Uuid(u));
            }
        }
    }

    Ok(result)
}

/// Apply the given modification
pub(super) fn apply_modification(
    task: &mut TaskMut,
    modification: &ResolvedModification,
) -> anyhow::Result<()> {
    // unwrap the "Resolved" promise
    let modification = &modification.0;

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
        task.add_tag(tag)?;
    }

    for tag in modification.remove_tags.iter() {
        task.remove_tag(tag)?;
    }

    if let Some(wait) = modification.wait {
        task.set_wait(wait)?;
    }

    if let Some(ref ann) = modification.annotate {
        task.add_annotation(Annotation {
            entry: Utc::now(),
            description: ann.into(),
        })?;
    }

    for tid in &modification.add_dependencies {
        if let TaskId::Uuid(u) = tid {
            task.add_dependency(*u)?;
        } else {
            // this Modification is resolved, so all TaskIds should
            // be the Uuid variant.
            unreachable!();
        }
    }

    for tid in &modification.remove_dependencies {
        if let TaskId::Uuid(u) = tid {
            task.remove_dependency(*u)?;
        } else {
            // this Modification is resolved, so all TaskIds should
            // be the Uuid variant.
            unreachable!();
        }
    }

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;
    use taskchampion::{Status, Uuid};

    #[test]
    fn test_resolve_modifications() {
        let mut replica = test_replica();
        let u1 = Uuid::new_v4();
        let t1 = replica.new_task(Status::Pending, "a task".into()).unwrap();
        replica.rebuild_working_set(true).unwrap();

        let modi = Modification {
            add_dependencies: set![TaskId::Uuid(u1), TaskId::WorkingSetId(1)],
            ..Default::default()
        };

        let res = resolve_modification(modi, &mut replica).unwrap();

        assert_eq!(
            res.0.add_dependencies,
            set![TaskId::Uuid(u1), TaskId::Uuid(t1.get_uuid())],
        );
    }

    #[test]
    fn test_resolve_task_ids_empty() {
        let mut replica = test_replica();

        assert_eq!(
            resolve_task_ids(&mut replica, HashSet::new()).unwrap(),
            HashSet::new()
        );
    }

    #[test]
    fn test_resolve_task_ids_all_uuids() {
        let mut replica = test_replica();
        let uuid = Uuid::new_v4();
        let tids = set![TaskId::Uuid(uuid)];
        assert_eq!(resolve_task_ids(&mut replica, tids.clone()).unwrap(), tids);
    }

    #[test]
    fn test_resolve_task_ids_working_set_not_found() {
        let mut replica = test_replica();
        let tids = set![TaskId::WorkingSetId(13)];
        assert_eq!(
            resolve_task_ids(&mut replica, tids.clone()).unwrap(),
            HashSet::new()
        );
    }

    #[test]
    fn test_resolve_task_ids_working_set() {
        let mut replica = test_replica();
        let t1 = replica.new_task(Status::Pending, "a task".into()).unwrap();
        let t2 = replica
            .new_task(Status::Pending, "another task".into())
            .unwrap();
        replica.rebuild_working_set(true).unwrap();
        let tids = set![TaskId::WorkingSetId(1), TaskId::WorkingSetId(2)];
        let resolved = set![TaskId::Uuid(t1.get_uuid()), TaskId::Uuid(t2.get_uuid())];
        assert_eq!(resolve_task_ids(&mut replica, tids).unwrap(), resolved);
    }

    #[test]
    fn test_resolve_task_ids_partial_not_found() {
        let mut replica = test_replica();
        let tids = set![TaskId::PartialUuid("abcd".into())];
        assert_eq!(
            resolve_task_ids(&mut replica, tids.clone()).unwrap(),
            HashSet::new()
        );
    }

    #[test]
    fn test_resolve_task_ids_partial() {
        let mut replica = test_replica();
        let t1 = replica.new_task(Status::Pending, "a task".into()).unwrap();
        let uuid_str = t1.get_uuid().to_string();
        let tids = set![TaskId::PartialUuid(uuid_str[..8].into())];
        let resolved = set![TaskId::Uuid(t1.get_uuid())];
        assert_eq!(resolve_task_ids(&mut replica, tids).unwrap(), resolved);
    }
}

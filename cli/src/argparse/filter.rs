use super::args::{arg_matching, id_list, minus_tag, plus_tag, TaskId};
use super::ArgList;
use crate::usage;
use nom::{branch::alt, combinator::*, multi::fold_many0, IResult};
use taskchampion::Status;

/// A filter represents a selection of a particular set of tasks.
///
/// A filter has a "universe" of tasks that might match, and a list of conditions
/// all of which tasks must match.  The universe can be a set of task IDs, or just
/// pending tasks, or all tasks.
#[derive(Debug, PartialEq, Default, Clone)]
pub(crate) struct Filter {
    /// A set of filter conditions, all of which must match a task in order for that task to be
    /// selected.
    pub(crate) conditions: Vec<Condition>,
}

/// A condition which tasks must match to be accepted by the filter.
#[derive(Debug, PartialEq, Clone)]
pub(crate) enum Condition {
    /// Task has the given tag
    HasTag(String),

    /// Task does not have the given tag
    NoTag(String),

    // TODO: add command-line syntax for this
    /// Task has the given status
    Status(Status),

    /// Task has one of the given IDs
    IdList(Vec<TaskId>),
}

impl Filter {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Filter> {
        fold_many0(
            alt((Self::id_list, Self::plus_tag, Self::minus_tag)),
            Filter {
                ..Default::default()
            },
            |acc, arg| acc.with_arg(arg),
        )(input)
    }

    /// fold multiple filter args into a single Filter instance
    fn with_arg(mut self, cond: Condition) -> Filter {
        if let Condition::IdList(mut id_list) = cond {
            // If there is already an IdList condition, concatenate this one
            // to it.  Thus multiple IdList command-line args represent an OR
            // operation.  This assumes that the filter is still being built
            // from command-line arguments and thus has at most one IdList
            // condition.
            if let Some(Condition::IdList(existing)) = self
                .conditions
                .iter_mut()
                .find(|c| matches!(c, Condition::IdList(_)))
            {
                existing.append(&mut id_list);
            } else {
                self.conditions.push(Condition::IdList(id_list));
            }
        } else {
            // all other command-line conditions are AND'd together
            self.conditions.push(cond);
        }
        self
    }

    /// combine this filter with another filter in an AND operation
    pub(crate) fn intersect(mut self, mut other: Filter) -> Filter {
        // simply concatenate the conditions
        self.conditions.append(&mut other.conditions);

        self
    }

    // parsers

    fn id_list(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_filterarg(input: Vec<TaskId>) -> Result<Condition, ()> {
            Ok(Condition::IdList(input))
        }
        map_res(arg_matching(id_list), to_filterarg)(input)
    }

    fn plus_tag(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_filterarg(input: &str) -> Result<Condition, ()> {
            Ok(Condition::HasTag(input.to_owned()))
        }
        map_res(arg_matching(plus_tag), to_filterarg)(input)
    }

    fn minus_tag(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_filterarg(input: &str) -> Result<Condition, ()> {
            Ok(Condition::NoTag(input.to_owned()))
        }
        map_res(arg_matching(minus_tag), to_filterarg)(input)
    }

    // usage

    pub(super) fn get_usage(u: &mut usage::Usage) {
        u.filters.push(usage::Filter {
            syntax: "TASKID[,TASKID,..]",
            summary: "Specific tasks",
            description: "
                Select only specific tasks.  Multiple tasks can be specified either separated by
                commas or as separate arguments.  Each task may be specfied by its working-set
                index (a small number) or by its UUID.  Partial UUIDs, broken on a hyphen, are
                also supported, such as `b5664ef8-423d` or `b5664ef8`.",
        });
        u.filters.push(usage::Filter {
            syntax: "+TAG",
            summary: "Tagged tasks",
            description: "
                Select tasks with the given tag.",
        });
        u.filters.push(usage::Filter {
            syntax: "-TAG",
            summary: "Un-tagged tasks",
            description: "
                Select tasks that do not have the given tag.",
        });
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_empty() {
        let (input, filter) = Filter::parse(argv![]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                ..Default::default()
            }
        );
    }

    #[test]
    fn test_id_list_single() {
        let (input, filter) = Filter::parse(argv!["1"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(1)])],
            }
        );
    }

    #[test]
    fn test_id_list_commas() {
        let (input, filter) = Filter::parse(argv!["1,2,3"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::WorkingSetId(2),
                    TaskId::WorkingSetId(3),
                ])],
            }
        );
    }

    #[test]
    fn test_id_list_multi_arg() {
        let (input, filter) = Filter::parse(argv!["1,2", "3,4"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::WorkingSetId(2),
                    TaskId::WorkingSetId(3),
                    TaskId::WorkingSetId(4),
                ])],
            }
        );
    }

    #[test]
    fn test_id_list_uuids() {
        let (input, filter) = Filter::parse(argv!["1,abcd1234"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::PartialUuid(s!("abcd1234")),
                ])],
            }
        );
    }

    #[test]
    fn test_tags() {
        let (input, filter) = Filter::parse(argv!["1", "+yes", "-no"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![
                    Condition::IdList(vec![TaskId::WorkingSetId(1),]),
                    Condition::HasTag("yes".into()),
                    Condition::NoTag("no".into()),
                ],
            }
        );
    }

    #[test]
    fn intersect_idlist_idlist() {
        let left = Filter::parse(argv!["1,2", "+yes"]).unwrap().1;
        let right = Filter::parse(argv!["2,3", "+no"]).unwrap().1;
        let both = left.intersect(right);
        assert_eq!(
            both,
            Filter {
                conditions: vec![
                    // from first filter
                    Condition::IdList(vec![TaskId::WorkingSetId(1), TaskId::WorkingSetId(2),]),
                    Condition::HasTag("yes".into()),
                    // from second filter
                    Condition::IdList(vec![TaskId::WorkingSetId(2), TaskId::WorkingSetId(3)]),
                    Condition::HasTag("no".into()),
                ],
            }
        );
    }

    #[test]
    fn intersect_idlist_alltasks() {
        let left = Filter::parse(argv!["1,2", "+yes"]).unwrap().1;
        let right = Filter::parse(argv!["+no"]).unwrap().1;
        let both = left.intersect(right);
        assert_eq!(
            both,
            Filter {
                conditions: vec![
                    // from first filter
                    Condition::IdList(vec![TaskId::WorkingSetId(1), TaskId::WorkingSetId(2),]),
                    Condition::HasTag("yes".into()),
                    // from second filter
                    Condition::HasTag("no".into()),
                ],
            }
        );
    }

    #[test]
    fn intersect_alltasks_alltasks() {
        let left = Filter::parse(argv!["+yes"]).unwrap().1;
        let right = Filter::parse(argv!["+no"]).unwrap().1;
        let both = left.intersect(right);
        assert_eq!(
            both,
            Filter {
                conditions: vec![
                    Condition::HasTag("yes".into()),
                    Condition::HasTag("no".into()),
                ],
            }
        );
    }
}

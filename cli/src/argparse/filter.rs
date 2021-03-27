use super::args::{arg_matching, id_list, minus_tag, plus_tag, status_colon, TaskId};
use super::ArgList;
use crate::usage;
use anyhow::bail;
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

    /// Task has the given status
    Status(Status),

    /// Task has one of the given IDs
    IdList(Vec<TaskId>),
}

impl Condition {
    fn parse(input: ArgList) -> IResult<ArgList, Condition> {
        alt((
            Self::parse_id_list,
            Self::parse_plus_tag,
            Self::parse_minus_tag,
            Self::parse_status,
        ))(input)
    }

    /// Parse a single condition string
    pub(crate) fn parse_str(input: &str) -> anyhow::Result<Condition> {
        let input = &[input];
        Ok(match Condition::parse(input) {
            Ok((&[], cond)) => cond,
            Ok(_) => unreachable!(), // input only has one element
            Err(nom::Err::Incomplete(_)) => unreachable!(),
            Err(nom::Err::Error(e)) => bail!("invalid filter condition: {:?}", e),
            Err(nom::Err::Failure(e)) => bail!("invalid filter condition: {:?}", e),
        })
    }

    fn parse_id_list(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_condition(input: Vec<TaskId>) -> Result<Condition, ()> {
            Ok(Condition::IdList(input))
        }
        map_res(arg_matching(id_list), to_condition)(input)
    }

    fn parse_plus_tag(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_condition(input: &str) -> Result<Condition, ()> {
            Ok(Condition::HasTag(input.to_owned()))
        }
        map_res(arg_matching(plus_tag), to_condition)(input)
    }

    fn parse_minus_tag(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_condition(input: &str) -> Result<Condition, ()> {
            Ok(Condition::NoTag(input.to_owned()))
        }
        map_res(arg_matching(minus_tag), to_condition)(input)
    }

    fn parse_status(input: ArgList) -> IResult<ArgList, Condition> {
        fn to_condition(input: Status) -> Result<Condition, ()> {
            Ok(Condition::Status(input))
        }
        map_res(arg_matching(status_colon), to_condition)(input)
    }
}

impl Filter {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Filter> {
        fold_many0(
            Condition::parse,
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
        u.filters.push(usage::Filter {
            syntax: "status:pending, status:completed, status:deleted",
            summary: "Task status",
            description: "
                Select tasks with the given status.",
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
    fn test_status() {
        let (input, filter) = Filter::parse(argv!["status:completed", "status:pending"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            filter,
            Filter {
                conditions: vec![
                    Condition::Status(Status::Completed),
                    Condition::Status(Status::Pending),
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

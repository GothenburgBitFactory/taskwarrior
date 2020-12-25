use super::args::{arg_matching, id_list, minus_tag, plus_tag, TaskId};
use super::ArgList;
use crate::usage;
use nom::{branch::alt, combinator::*, multi::fold_many0, IResult};

/// A filter represents a selection of a particular set of tasks.
///
/// A filter has a "universe" of tasks that might match, and a list of conditions
/// all of which tasks must match.  The universe can be a set of task IDs, or just
/// pending tasks, or all tasks.
#[derive(Debug, PartialEq, Default, Clone)]
pub(crate) struct Filter {
    /// The universe of tasks from which this filter can select
    pub(crate) universe: Universe,

    /// A set of filter conditions, all of which must match a task in order for that task to be
    /// selected.
    pub(crate) conditions: Vec<Condition>,
}

/// The universe of tasks over which a filter should be applied.
#[derive(Debug, PartialEq, Clone)]
pub(crate) enum Universe {
    /// Only the identified tasks.  Note that this may contain duplicates.
    IdList(Vec<TaskId>),
    /// All tasks in the task database
    AllTasks,
    /// Only pending tasks (or as an approximation, the working set)
    #[allow(dead_code)] // currently only used in tests
    PendingTasks,
}

impl Universe {
    /// Testing shorthand to construct a simple universe
    #[cfg(test)]
    pub(super) fn for_ids(mut ids: Vec<usize>) -> Self {
        Universe::IdList(ids.drain(..).map(|id| TaskId::WorkingSetId(id)).collect())
    }
}

impl Default for Universe {
    fn default() -> Self {
        Self::AllTasks
    }
}

/// A condition which tasks must match to be accepted by the filter.
#[derive(Debug, PartialEq, Clone)]
pub(crate) enum Condition {
    /// Task has the given tag
    HasTag(String),

    /// Task does not have the given tag
    NoTag(String),
}

/// Internal struct representing a parsed filter argument
enum FilterArg {
    IdList(Vec<TaskId>),
    Condition(Condition),
}

impl Filter {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Filter> {
        fold_many0(
            alt((Self::id_list, Self::plus_tag, Self::minus_tag)),
            Filter {
                ..Default::default()
            },
            Self::fold_args,
        )(input)
    }

    /// fold multiple filter args into a single Filter instance
    fn fold_args(mut acc: Filter, mod_arg: FilterArg) -> Filter {
        match mod_arg {
            FilterArg::IdList(mut id_list) => {
                // If any IDs are specified, then the filter's universe
                // is those IDs.  If there are already IDs, append to the
                // list.
                if let Universe::IdList(ref mut existing) = acc.universe {
                    existing.append(&mut id_list);
                } else {
                    acc.universe = Universe::IdList(id_list);
                }
            }
            FilterArg::Condition(cond) => {
                acc.conditions.push(cond);
            }
        }
        acc
    }

    fn id_list(input: ArgList) -> IResult<ArgList, FilterArg> {
        fn to_filterarg(input: Vec<TaskId>) -> Result<FilterArg, ()> {
            Ok(FilterArg::IdList(input))
        }
        map_res(arg_matching(id_list), to_filterarg)(input)
    }

    fn plus_tag(input: ArgList) -> IResult<ArgList, FilterArg> {
        fn to_filterarg(input: &str) -> Result<FilterArg, ()> {
            Ok(FilterArg::Condition(Condition::HasTag(input.to_owned())))
        }
        map_res(arg_matching(plus_tag), to_filterarg)(input)
    }

    fn minus_tag(input: ArgList) -> IResult<ArgList, FilterArg> {
        fn to_filterarg(input: &str) -> Result<FilterArg, ()> {
            Ok(FilterArg::Condition(Condition::NoTag(input.to_owned())))
        }
        map_res(arg_matching(minus_tag), to_filterarg)(input)
    }

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
                universe: Universe::IdList(vec![TaskId::WorkingSetId(1)]),
                ..Default::default()
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
                universe: Universe::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::WorkingSetId(2),
                    TaskId::WorkingSetId(3),
                ]),
                ..Default::default()
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
                universe: Universe::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::WorkingSetId(2),
                    TaskId::WorkingSetId(3),
                    TaskId::WorkingSetId(4),
                ]),
                ..Default::default()
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
                universe: Universe::IdList(vec![
                    TaskId::WorkingSetId(1),
                    TaskId::PartialUuid(s!("abcd1234")),
                ]),
                ..Default::default()
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
                universe: Universe::IdList(vec![TaskId::WorkingSetId(1),]),
                conditions: vec![
                    Condition::HasTag("yes".into()),
                    Condition::NoTag("no".into()),
                ],
                ..Default::default()
            }
        );
    }
}

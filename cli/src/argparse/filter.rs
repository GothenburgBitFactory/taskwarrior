use super::args::{arg_matching, id_list, TaskId};
use super::ArgList;
use nom::{combinator::*, multi::fold_many0, IResult};

/// A filter represents a selection of a particular set of tasks.
///
/// A filter has a "universe" of tasks that might match, and a list of conditions
/// all of which tasks must match.  The universe can be a set of task IDs, or just
/// pending tasks, or all tasks.
#[derive(Debug, PartialEq, Default, Clone)]
pub(crate) struct Filter {
    /// A list of numeric IDs or prefixes of UUIDs
    pub(crate) universe: Universe,
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

/// Internal struct representing a parsed filter argument
enum FilterArg {
    IdList(Vec<TaskId>),
}

impl Filter {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Filter> {
        fold_many0(
            Self::id_list,
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
        }
        acc
    }

    fn id_list(input: ArgList) -> IResult<ArgList, FilterArg> {
        fn to_filterarg(input: Vec<TaskId>) -> Result<FilterArg, ()> {
            Ok(FilterArg::IdList(input))
        }
        map_res(arg_matching(id_list), to_filterarg)(input)
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
                    TaskId::PartialUuid("abcd1234".to_owned()),
                ]),
                ..Default::default()
            }
        );
    }
}

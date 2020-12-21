use super::args::{arg_matching, id_list};
use super::ArgList;
use nom::{combinator::*, multi::fold_many0, IResult};

/// A filter represents a selection of a particular set of tasks.
#[derive(Debug, PartialEq, Default, Clone)]
pub(crate) struct Filter {
    /// A list of numeric IDs or prefixes of UUIDs
    pub(crate) id_list: Option<Vec<String>>,
}

enum FilterArg {
    IdList(Vec<String>),
}

impl Filter {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Filter> {
        fn fold(mut acc: Filter, mod_arg: FilterArg) -> Filter {
            match mod_arg {
                FilterArg::IdList(mut id_list) => {
                    if let Some(ref mut existing) = acc.id_list {
                        // given multiple ID lists, concatenate them to represent
                        // an "OR" between them.
                        existing.append(&mut id_list);
                    } else {
                        acc.id_list = Some(id_list);
                    }
                }
            }
            acc
        }
        fold_many0(
            Self::id_list,
            Filter {
                ..Default::default()
            },
            fold,
        )(input)
    }

    fn id_list(input: ArgList) -> IResult<ArgList, FilterArg> {
        fn to_filterarg(mut input: Vec<&str>) -> Result<FilterArg, ()> {
            Ok(FilterArg::IdList(
                input.drain(..).map(str::to_owned).collect(),
            ))
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
                id_list: Some(vec!["1".to_owned()]),
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
                id_list: Some(vec!["1".to_owned(), "2".to_owned(), "3".to_owned()]),
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
                id_list: Some(vec!["1".to_owned(), "abcd1234".to_owned()]),
                ..Default::default()
            }
        );
    }
}

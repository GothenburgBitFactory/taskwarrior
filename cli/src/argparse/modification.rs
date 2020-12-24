use super::args::{any, arg_matching, minus_tag, plus_tag};
use super::ArgList;
use crate::usage;
use nom::{branch::alt, combinator::*, multi::fold_many0, IResult};
use std::collections::HashSet;
use taskchampion::Status;

#[derive(Debug, PartialEq, Clone)]
pub enum DescriptionMod {
    /// do not change the description
    None,

    /// Prepend the given value to the description, with a space separator
    Prepend(String),

    /// Append the given value to the description, with a space separator
    Append(String),

    /// Set the description
    Set(String),
}

impl Default for DescriptionMod {
    fn default() -> Self {
        Self::None
    }
}

/// A modification represents a change to a task: adding or removing tags, setting the
/// description, and so on.
#[derive(Debug, PartialEq, Clone, Default)]
pub struct Modification {
    /// Change the description
    pub description: DescriptionMod,

    /// Set the status
    pub status: Option<Status>,

    /// Set the "active" state, that is, start (true) or stop (false) the task.
    pub active: Option<bool>,

    /// Add tags
    pub add_tags: HashSet<String>,

    /// Remove tags
    pub remove_tags: HashSet<String>,
}

/// A single argument that is part of a modification, used internally to this module
enum ModArg<'a> {
    Description(&'a str),
    PlusTag(&'a str),
    MinusTag(&'a str),
}

impl Modification {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Modification> {
        fn fold(mut acc: Modification, mod_arg: ModArg) -> Modification {
            match mod_arg {
                ModArg::Description(description) => {
                    if let DescriptionMod::Set(existing) = acc.description {
                        acc.description =
                            DescriptionMod::Set(format!("{} {}", existing, description));
                    } else {
                        acc.description = DescriptionMod::Set(description.to_string());
                    }
                }
                ModArg::PlusTag(tag) => {
                    acc.add_tags.insert(tag.to_owned());
                }
                ModArg::MinusTag(tag) => {
                    acc.remove_tags.insert(tag.to_owned());
                }
            }
            acc
        }
        fold_many0(
            alt((
                Self::plus_tag,
                Self::minus_tag,
                // this must come last
                Self::description,
            )),
            Modification {
                ..Default::default()
            },
            fold,
        )(input)
    }

    fn description(input: ArgList) -> IResult<ArgList, ModArg> {
        fn to_modarg(input: &str) -> Result<ModArg, ()> {
            Ok(ModArg::Description(input))
        }
        map_res(arg_matching(any), to_modarg)(input)
    }

    fn plus_tag(input: ArgList) -> IResult<ArgList, ModArg> {
        fn to_modarg(input: &str) -> Result<ModArg, ()> {
            Ok(ModArg::PlusTag(input))
        }
        map_res(arg_matching(plus_tag), to_modarg)(input)
    }

    fn minus_tag(input: ArgList) -> IResult<ArgList, ModArg> {
        fn to_modarg(input: &str) -> Result<ModArg, ()> {
            Ok(ModArg::MinusTag(input))
        }
        map_res(arg_matching(minus_tag), to_modarg)(input)
    }

    pub(super) fn get_usage(u: &mut usage::Usage) {
        u.modifications.push(usage::Modification {
            syntax: "DESCRIPTION",
            summary: "Set description",
            description: "
                Set the task description.  Multiple arguments are combined into a single
                space-separated description.",
        });
        u.modifications.push(usage::Modification {
            syntax: "+TAG",
            summary: "Tag task",
            description: "
                Add the given tag to the task.",
        });
        u.modifications.push(usage::Modification {
            syntax: "-TAG",
            summary: "Un-tag task",
            description: "
                Remove the given tag from the task.",
        });
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_empty() {
        let (input, modification) = Modification::parse(argv![]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            modification,
            Modification {
                ..Default::default()
            }
        );
    }

    #[test]
    fn test_single_arg_description() {
        let (input, modification) = Modification::parse(argv!["newdesc"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            modification,
            Modification {
                description: DescriptionMod::Set("newdesc".to_owned()),
                ..Default::default()
            }
        );
    }

    #[test]
    fn test_add_tags() {
        let (input, modification) = Modification::parse(argv!["+abc", "+def"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            modification,
            Modification {
                add_tags: set!["abc".to_owned(), "def".to_owned()],
                ..Default::default()
            }
        );
    }

    #[test]
    fn test_multi_arg_description() {
        let (input, modification) = Modification::parse(argv!["new", "desc", "fun"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            modification,
            Modification {
                description: DescriptionMod::Set("new desc fun".to_owned()),
                ..Default::default()
            }
        );
    }

    #[test]
    fn test_multi_arg_description_and_tags() {
        let (input, modification) =
            Modification::parse(argv!["new", "+next", "desc", "-daytime", "fun"]).unwrap();
        assert_eq!(input.len(), 0);
        assert_eq!(
            modification,
            Modification {
                description: DescriptionMod::Set("new desc fun".to_owned()),
                add_tags: set!["next".to_owned()],
                remove_tags: set!["daytime".to_owned()],
                ..Default::default()
            }
        );
    }
}

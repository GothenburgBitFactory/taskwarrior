use super::args::{any, arg_matching};
use super::ArgList;
use nom::{combinator::*, multi::fold_many0, IResult};
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

    /// Set the "active" status, that is, start (true) or stop (false) the task.
    pub active: Option<bool>,
}

/// A single argument that is part of a modification, used internally to this module
enum ModArg<'a> {
    Description(&'a str),
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
            }
            acc
        }
        fold_many0(
            Self::description,
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
}

use super::args::{any, arg_matching, literal};
use super::ArgList;
use crate::usage;
use nom::{branch::alt, combinator::*, sequence::*, IResult};

#[derive(Debug, PartialEq)]
/// A config operation
pub(crate) enum ConfigOperation {
    /// Set a configuration value
    Set(String, String),
    /// Show configuration path
    Path,
}

impl ConfigOperation {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, ConfigOperation> {
        fn set_to_op(input: (&str, &str, &str)) -> Result<ConfigOperation, ()> {
            Ok(ConfigOperation::Set(input.1.to_owned(), input.2.to_owned()))
        }
        fn path_to_op(_: &str) -> Result<ConfigOperation, ()> {
            Ok(ConfigOperation::Path)
        }
        alt((
            map_res(
                tuple((
                    arg_matching(literal("set")),
                    arg_matching(any),
                    arg_matching(any),
                )),
                set_to_op,
            ),
            map_res(arg_matching(literal("path")), path_to_op),
        ))(input)
    }

    pub(super) fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "config set",
            syntax: "config set <key> <value>",
            summary: "Set a configuration value",
            description: "Update Taskchampion configuration file to set key = value",
        });
    }
}

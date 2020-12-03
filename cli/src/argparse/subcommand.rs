use super::args::*;
use super::{ArgList, DescriptionMod, Filter, Modification, Report};
use nom::{branch::alt, combinator::*, sequence::*, IResult};
use taskchampion::Status;

/// A subcommand is the specific operation that the CLI should execute.
#[derive(Debug, PartialEq)]
pub(crate) enum Subcommand {
    /// Display the tool version
    Version,

    /// Display the help output
    Help {
        /// Give the summary help (fitting on a few lines)
        summary: bool,
    },

    /// Add a new task
    Add {
        modification: Modification,
    },

    /// Modify existing tasks
    Modify {
        filter: Filter,
        modification: Modification,
    },

    /// Lists (reports)
    List {
        report: Report,
    },

    /// Per-task information (typically one task)
    Info {
        filter: Filter,
        debug: bool,
    },

    /// Basic operations without args
    Gc,
    Sync,
}

impl Subcommand {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        alt((
            Self::version,
            Self::help,
            Self::add,
            Self::modify_prepend_append,
            Self::start_stop_done,
            Self::list,
            Self::info,
            Self::gc,
            Self::sync,
        ))(input)
    }

    fn version(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Version)
        }
        map_res(
            alt((
                arg_matching(literal("version")),
                arg_matching(literal("--version")),
            )),
            to_subcommand,
        )(input)
    }

    fn help(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Help {
                summary: input == "-h",
            })
        }
        map_res(
            alt((
                arg_matching(literal("help")),
                arg_matching(literal("--help")),
                arg_matching(literal("-h")),
            )),
            to_subcommand,
        )(input)
    }

    fn add(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (&str, Modification)) -> Result<Subcommand, ()> {
            Ok(Subcommand::Add {
                modification: input.1,
            })
        }
        map_res(
            pair(arg_matching(literal("add")), Modification::parse),
            to_subcommand,
        )(input)
    }

    fn modify_prepend_append(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (Filter, &str, Modification)) -> Result<Subcommand, ()> {
            let filter = input.0;
            let mut modification = input.2;

            match input.1 {
                "prepend" => {
                    if let DescriptionMod::Set(s) = modification.description {
                        modification.description = DescriptionMod::Prepend(s)
                    }
                }
                "append" => {
                    if let DescriptionMod::Set(s) = modification.description {
                        modification.description = DescriptionMod::Append(s)
                    }
                }
                _ => {}
            }

            Ok(Subcommand::Modify {
                filter,
                modification,
            })
        }
        map_res(
            tuple((
                Filter::parse,
                alt((
                    arg_matching(literal("modify")),
                    arg_matching(literal("prepend")),
                    arg_matching(literal("append")),
                )),
                Modification::parse,
            )),
            to_subcommand,
        )(input)
    }

    fn start_stop_done(input: ArgList) -> IResult<ArgList, Subcommand> {
        // start, stop, and done are special cases of modify
        fn to_subcommand(input: (Filter, &str, Modification)) -> Result<Subcommand, ()> {
            let filter = input.0;
            let mut modification = input.2;
            match input.1 {
                "start" => modification.active = Some(true),
                "stop" => modification.active = Some(false),
                "done" => modification.status = Some(Status::Completed),
                _ => unreachable!(),
            }
            Ok(Subcommand::Modify {
                filter,
                modification,
            })
        }
        map_res(
            tuple((
                Filter::parse,
                alt((
                    arg_matching(literal("start")),
                    arg_matching(literal("stop")),
                    arg_matching(literal("done")),
                )),
                Modification::parse,
            )),
            to_subcommand,
        )(input)
    }

    fn list(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (Report, &str)) -> Result<Subcommand, ()> {
            Ok(Subcommand::List { report: input.0 })
        }
        map_res(
            pair(Report::parse, arg_matching(literal("list"))),
            to_subcommand,
        )(input)
    }

    fn info(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (Filter, &str)) -> Result<Subcommand, ()> {
            let debug = input.1 == "debug";
            Ok(Subcommand::Info {
                filter: input.0,
                debug,
            })
        }
        map_res(
            pair(
                Filter::parse,
                alt((
                    arg_matching(literal("info")),
                    arg_matching(literal("debug")),
                )),
            ),
            to_subcommand,
        )(input)
    }

    fn gc(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Gc)
        }
        map_res(arg_matching(literal("gc")), to_subcommand)(input)
    }

    fn sync(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Sync)
        }
        map_res(arg_matching(literal("sync")), to_subcommand)(input)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    const EMPTY: Vec<&str> = vec![];

    #[test]
    fn test_version() {
        assert_eq!(
            Subcommand::parse(argv!["version"]).unwrap(),
            (&EMPTY[..], Subcommand::Version)
        );
    }

    #[test]
    fn test_dd_version() {
        assert_eq!(
            Subcommand::parse(argv!["--version"]).unwrap(),
            (&EMPTY[..], Subcommand::Version)
        );
    }

    #[test]
    fn test_d_h() {
        assert_eq!(
            Subcommand::parse(argv!["-h"]).unwrap(),
            (&EMPTY[..], Subcommand::Help { summary: true })
        );
    }

    #[test]
    fn test_help() {
        assert_eq!(
            Subcommand::parse(argv!["help"]).unwrap(),
            (&EMPTY[..], Subcommand::Help { summary: false })
        );
    }

    #[test]
    fn test_dd_help() {
        assert_eq!(
            Subcommand::parse(argv!["--help"]).unwrap(),
            (&EMPTY[..], Subcommand::Help { summary: false })
        );
    }

    #[test]
    fn test_add_description() {
        let subcommand = Subcommand::Add {
            modification: Modification {
                description: DescriptionMod::Set("foo".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["add", "foo"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_add_description_multi() {
        let subcommand = Subcommand::Add {
            modification: Modification {
                description: DescriptionMod::Set("foo bar".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["add", "foo", "bar"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_modify_description_multi() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                description: DescriptionMod::Set("foo bar".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "modify", "foo", "bar"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_append() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                description: DescriptionMod::Append("foo bar".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "append", "foo", "bar"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_prepend() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                description: DescriptionMod::Prepend("foo bar".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "prepend", "foo", "bar"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_done() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                status: Some(Status::Completed),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "done"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_done_modify() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                description: DescriptionMod::Set("now-finished".to_owned()),
                status: Some(Status::Completed),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "done", "now-finished"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_start() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                active: Some(true),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "start"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_start_modify() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                active: Some(true),
                description: DescriptionMod::Set("mod".to_owned()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "start", "mod"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_stop() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                active: Some(false),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "stop"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_stop_modify() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                id_list: Some(vec!["123".to_owned()]),
            },
            modification: Modification {
                description: DescriptionMod::Set("mod".to_owned()),
                active: Some(false),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "stop", "mod"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_list() {
        let subcommand = Subcommand::List {
            report: Report {
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["list"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_list_filter() {
        let subcommand = Subcommand::List {
            report: Report {
                filter: Filter {
                    id_list: Some(vec!["12".to_owned(), "13".to_owned()]),
                },
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["12,13", "list"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_info_filter() {
        let subcommand = Subcommand::Info {
            debug: false,
            filter: Filter {
                id_list: Some(vec!["12".to_owned(), "13".to_owned()]),
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["12,13", "info"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_debug_filter() {
        let subcommand = Subcommand::Info {
            debug: true,
            filter: Filter {
                id_list: Some(vec!["12".to_owned()]),
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["12", "debug"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_gc() {
        let subcommand = Subcommand::Gc;
        assert_eq!(
            Subcommand::parse(argv!["gc"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_gc_extra_args() {
        let subcommand = Subcommand::Gc;
        assert_eq!(
            Subcommand::parse(argv!["gc", "foo"]).unwrap(),
            (&vec!["foo"][..], subcommand)
        );
    }

    #[test]
    fn test_sync() {
        let subcommand = Subcommand::Sync;
        assert_eq!(
            Subcommand::parse(argv!["sync"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }
}

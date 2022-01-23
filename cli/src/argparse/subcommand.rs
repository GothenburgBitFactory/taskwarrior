use super::args::*;
use super::{ArgList, ConfigOperation, DescriptionMod, Filter, Modification};
use crate::usage;
use nom::{branch::alt, combinator::*, sequence::*, IResult};
use taskchampion::Status;

// IMPLEMENTATION NOTE:
//
// For each variant of Subcommand, there is a private, empty type of the same name with a `parse`
// method and a `get_usage` method.  The parse methods may handle several subcommands, but always
// produce the variant of the same name as the type.
//
// This organization helps to gather the parsing and usage information into
// comprehensible chunks of code, to ensure that everything is documented.

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

    /// Manipulate configuration
    Config {
        config_operation: ConfigOperation,
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
    Report {
        /// The name of the report to show
        report_name: String,

        /// Additional filter terms beyond those in the report
        filter: Filter,
    },

    /// Per-task information (typically one task)
    Info {
        filter: Filter,
        debug: bool,
    },

    /// Basic operations without args
    Gc,
    Sync,
    ImportTW,
    ImportTDB2 {
        path: String,
    },
    Undo,
}

impl Subcommand {
    pub(super) fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        all_consuming(alt((
            Version::parse,
            Help::parse,
            Config::parse,
            Add::parse,
            Modify::parse,
            Info::parse,
            Gc::parse,
            Sync::parse,
            ImportTW::parse,
            ImportTDB2::parse,
            Undo::parse,
            // This must come last since it accepts arbitrary report names
            Report::parse,
        )))(input)
    }

    pub(super) fn get_usage(u: &mut usage::Usage) {
        Version::get_usage(u);
        Help::get_usage(u);
        Config::get_usage(u);
        Add::get_usage(u);
        Modify::get_usage(u);
        Info::get_usage(u);
        Gc::get_usage(u);
        Sync::get_usage(u);
        ImportTW::get_usage(u);
        ImportTDB2::get_usage(u);
        Undo::get_usage(u);
        Report::get_usage(u);
    }
}

struct Version;

impl Version {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
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

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "version",
            syntax: "version",
            summary: "Show the TaskChampion version",
            description: "Show the version of the TaskChampion binary",
        });
    }
}

struct Help;

impl Help {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
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

    fn get_usage(_u: &mut usage::Usage) {}
}

struct Config;

impl Config {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (&str, ConfigOperation)) -> Result<Subcommand, ()> {
            Ok(Subcommand::Config {
                config_operation: input.1,
            })
        }
        map_res(
            tuple((arg_matching(literal("config")), ConfigOperation::parse)),
            to_subcommand,
        )(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        ConfigOperation::get_usage(u);
    }
}

struct Add;

impl Add {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
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

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "add",
            syntax: "add [modification]",
            summary: "Add a new task",
            description: "
                Add a new, pending task to the list of tasks.  The modification must include a
                description.",
        });
    }
}

struct Modify;

impl Modify {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
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
                "start" => modification.active = Some(true),
                "stop" => modification.active = Some(false),
                "done" => modification.status = Some(Status::Completed),
                "annotate" => {
                    // what would be parsed as a description is, here, used as the annotation
                    if let DescriptionMod::Set(s) = modification.description {
                        modification.description = DescriptionMod::None;
                        modification.annotate = Some(s);
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
                Filter::parse1,
                alt((
                    arg_matching(literal("modify")),
                    arg_matching(literal("prepend")),
                    arg_matching(literal("append")),
                    arg_matching(literal("start")),
                    arg_matching(literal("stop")),
                    arg_matching(literal("done")),
                    arg_matching(literal("annotate")),
                )),
                Modification::parse,
            )),
            to_subcommand,
        )(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "modify",
            syntax: "<filter> modify [modification]",
            summary: "Modify tasks",
            description: "
                Modify all tasks matching the required filter.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "prepend",
            syntax: "<filter> prepend [modification]",
            summary: "Prepend task description",
            description: "
                Modify all tasks matching the required filter by inserting the given description before each
                task's description.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "append",
            syntax: "<filter> append [modification]",
            summary: "Append task description",
            description: "
                Modify all tasks matching the required filter by adding the given description to the end
                of each task's description.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "start",
            syntax: "<filter> start [modification]",
            summary: "Start tasks",
            description: "
                Start all tasks matching the required filter, additionally applying any given modifications."
        });
        u.subcommands.push(usage::Subcommand {
            name: "stop",
            syntax: "<filter> stop [modification]",
            summary: "Stop tasks",
            description: "
                Stop all tasks matching the required filter, additionally applying any given modifications.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "done",
            syntax: "<filter> done [modification]",
            summary: "Mark tasks as completed",
            description: "
                Mark all tasks matching the required filter as completed, additionally applying any given
                modifications.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "annotate",
            syntax: "<filter> annotate [modification]",
            summary: "Mark tasks as completed",
            description: "
                Add an annotation to all tasks matching the required filter.",
        });
    }
}

struct Report;

impl Report {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(filter: Filter, report_name: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Report {
                filter,
                report_name: report_name.to_owned(),
            })
        }
        // allow the filter expression before or after the report name
        alt((
            map_res(pair(arg_matching(report_name), Filter::parse0), |input| {
                to_subcommand(input.1, input.0)
            }),
            map_res(pair(Filter::parse0, arg_matching(report_name)), |input| {
                to_subcommand(input.0, input.1)
            }),
            // default to a "next" report
            map_res(Filter::parse0, |input| to_subcommand(input, "next")),
        ))(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "report",
            syntax: "[filter] [report-name] *or* [report-name] [filter]",
            summary: "Show a report",
            description: "
                Show the named report, including only tasks matching the filter",
        });
        u.subcommands.push(usage::Subcommand {
            name: "next",
            syntax: "[filter]",
            summary: "Show the 'next' report",
            description: "
                Show the report named 'next', including only tasks matching the filter",
        });
    }
}

struct Info;

impl Info {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (Filter, &str)) -> Result<Subcommand, ()> {
            let debug = input.1 == "debug";
            Ok(Subcommand::Info {
                filter: input.0,
                debug,
            })
        }
        map_res(
            pair(
                Filter::parse1,
                alt((
                    arg_matching(literal("info")),
                    arg_matching(literal("debug")),
                )),
            ),
            to_subcommand,
        )(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "info",
            syntax: "[filter] info",
            summary: "Show tasks",
            description: " Show information about all tasks matching the fiter.",
        });
        u.subcommands.push(usage::Subcommand {
            name: "debug",
            syntax: "[filter] debug",
            summary: "Show task debug details",
            description: " Show all key/value properties of the tasks matching the fiter.",
        });
    }
}

struct Gc;

impl Gc {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Gc)
        }
        map_res(arg_matching(literal("gc")), to_subcommand)(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "gc",
            syntax: "gc",
            summary: "Perform 'garbage collection'",
            description: "
                Perform 'garbage collection'.  This refreshes the list of pending tasks
                and their short id's.",
        });
    }
}

struct Sync;

impl Sync {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Sync)
        }
        map_res(arg_matching(literal("sync")), to_subcommand)(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "sync",
            syntax: "sync",
            summary: "Synchronize this replica",
            description: "
                Synchronize this replica locally or against a remote server, as configured.

                Synchronization is a critical part of maintaining the task database, and should
                be done regularly, even if only locally.  It is typically run in a crontask.",
        })
    }
}

struct ImportTW;

impl ImportTW {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::ImportTW)
        }
        map_res(arg_matching(literal("import-tw")), to_subcommand)(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "import-tw",
            syntax: "import-tw",
            summary: "Import tasks from TaskWarrior export",
            description: "
                Import tasks into this replica.

                The tasks must be provided in the TaskWarrior JSON format on stdin.  If tasks
                in the import already exist, they are 'merged'.

                Because TaskChampion lacks the information about the types of UDAs that is stored
                in the TaskWarrior configuration, UDA values are imported as simple strings, in the
                format they appear in the JSON export.  This may cause undesirable results.
                ",
        })
    }
}

struct ImportTDB2;

impl ImportTDB2 {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(input: (&str, &str)) -> Result<Subcommand, ()> {
            Ok(Subcommand::ImportTDB2 {
                path: input.1.into(),
            })
        }
        map_res(
            pair(arg_matching(literal("import-tdb2")), arg_matching(any)),
            to_subcommand,
        )(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "import-tdb2",
            syntax: "import-tdb2 <directory>",
            summary: "Import tasks from the TaskWarrior data directory",
            description: "
                Import tasks into this replica from a TaskWarrior data directory.  If tasks in the
                import already exist, they are 'merged'.  This mode of import supports UDAs better
                than the `import` subcommand, but requires access to the \"raw\" TaskWarrior data.

                This command supports task directories written by TaskWarrior-2.6.1 or later.
                ",
        })
    }
}

struct Undo;

impl Undo {
    fn parse(input: ArgList) -> IResult<ArgList, Subcommand> {
        fn to_subcommand(_: &str) -> Result<Subcommand, ()> {
            Ok(Subcommand::Undo)
        }
        map_res(arg_matching(literal("undo")), to_subcommand)(input)
    }

    fn get_usage(u: &mut usage::Usage) {
        u.subcommands.push(usage::Subcommand {
            name: "undo",
            syntax: "undo",
            summary: "Undo the latest change made on this replica",
            description: "
                Undo the latest change made on this replica.

                Changes cannot be undone once they have been synchronized.",
        })
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::argparse::Condition;
    use pretty_assertions::assert_eq;

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
    fn test_config_set() {
        assert_eq!(
            Subcommand::parse(argv!["config", "set", "x", "y"]).unwrap(),
            (
                &EMPTY[..],
                Subcommand::Config {
                    config_operation: ConfigOperation::Set("x".to_owned(), "y".to_owned())
                }
            )
        );
    }

    #[test]
    fn test_add_description() {
        let subcommand = Subcommand::Add {
            modification: Modification {
                description: DescriptionMod::Set(s!("foo")),
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
                description: DescriptionMod::Set(s!("foo bar")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                description: DescriptionMod::Set(s!("foo bar")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                description: DescriptionMod::Append(s!("foo bar")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                description: DescriptionMod::Prepend(s!("foo bar")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                description: DescriptionMod::Set(s!("now-finished")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                active: Some(true),
                description: DescriptionMod::Set(s!("mod")),
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                description: DescriptionMod::Set(s!("mod")),
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
    fn test_annotate() {
        let subcommand = Subcommand::Modify {
            filter: Filter {
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(123)])],
            },
            modification: Modification {
                annotate: Some("sent invoice".into()),
                ..Default::default()
            },
        };
        assert_eq!(
            Subcommand::parse(argv!["123", "annotate", "sent", "invoice"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_report() {
        let subcommand = Subcommand::Report {
            filter: Default::default(),
            report_name: "myreport".to_owned(),
        };
        assert_eq!(
            Subcommand::parse(argv!["myreport"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_report_filter_before() {
        let subcommand = Subcommand::Report {
            filter: Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(12),
                    TaskId::WorkingSetId(13),
                ])],
            },
            report_name: "foo".to_owned(),
        };
        assert_eq!(
            Subcommand::parse(argv!["12,13", "foo"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_report_filter_after() {
        let subcommand = Subcommand::Report {
            filter: Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(12),
                    TaskId::WorkingSetId(13),
                ])],
            },
            report_name: "foo".to_owned(),
        };
        assert_eq!(
            Subcommand::parse(argv!["foo", "12,13"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_report_filter_next() {
        let subcommand = Subcommand::Report {
            filter: Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(12),
                    TaskId::WorkingSetId(13),
                ])],
            },
            report_name: "next".to_owned(),
        };
        assert_eq!(
            Subcommand::parse(argv!["12,13"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_report_next() {
        let subcommand = Subcommand::Report {
            filter: Filter {
                ..Default::default()
            },
            report_name: "next".to_owned(),
        };
        assert_eq!(
            Subcommand::parse(argv![]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_info_filter() {
        let subcommand = Subcommand::Info {
            debug: false,
            filter: Filter {
                conditions: vec![Condition::IdList(vec![
                    TaskId::WorkingSetId(12),
                    TaskId::WorkingSetId(13),
                ])],
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
                conditions: vec![Condition::IdList(vec![TaskId::WorkingSetId(12)])],
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
        assert!(Subcommand::parse(argv!["gc", "foo"]).is_err());
    }

    #[test]
    fn test_sync() {
        let subcommand = Subcommand::Sync;
        assert_eq!(
            Subcommand::parse(argv!["sync"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }

    #[test]
    fn test_undo() {
        let subcommand = Subcommand::Undo;
        assert_eq!(
            Subcommand::parse(argv!["undo"]).unwrap(),
            (&EMPTY[..], subcommand)
        );
    }
}

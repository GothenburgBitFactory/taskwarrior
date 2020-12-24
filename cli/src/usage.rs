//! This module handles creation of CLI usage documents (--help, manpages, etc.) in
//! a way that puts the source of that documentation near its implementation.

use crate::argparse;
use std::io::{Result, Write};
use textwrap::indent;

/// A top-level structure containing usage/help information for the entire CLI.
#[derive(Debug, Default)]
pub(crate) struct Usage {
    pub(crate) subcommands: Vec<Subcommand>,
    pub(crate) filters: Vec<Filter>,
    pub(crate) modifications: Vec<Modification>,
}

impl Usage {
    /// Get a new, completely-filled-out usage object
    pub(crate) fn new() -> Self {
        let mut rv = Self {
            ..Default::default()
        };

        argparse::get_usage(&mut rv);

        // TODO: sort subcommands

        rv
    }

    /// Write this usage to the given output as a help string, writing a short version if `summary`
    /// is true.
    pub(crate) fn write_help<W: Write>(
        &self,
        mut w: W,
        command_name: String,
        summary: bool,
    ) -> Result<()> {
        write!(
            w,
            "TaskChampion {}: Personal task-tracking\n\n",
            env!("CARGO_PKG_VERSION")
        )?;
        writeln!(w, "USAGE:\n  {} [args]\n", command_name)?;
        writeln!(w, "TaskChampion subcommands:")?;
        for subcommand in self.subcommands.iter() {
            subcommand.write_help(&mut w, summary)?;
        }
        write!(w, "Filter Expressions:\n\n")?;
        write!(w, "Where [filter] appears above, zero or more of the following arguments can be used to limit\n")?;
        write!(w, "the tasks concerned.\n\n")?;
        for filter in self.filters.iter() {
            filter.write_help(&mut w, summary)?;
        }
        write!(w, "Modifications:\n\n")?;
        write!(w, "Where [modification] appears above, zero or more of the following arguments can be used\n")?;
        write!(w, "to modify the selected tasks.\n\n")?;
        for modification in self.modifications.iter() {
            modification.write_help(&mut w, summary)?;
        }
        if !summary {
            writeln!(w, "\nSee `task help` for more detail")?;
        }
        Ok(())
    }
}

/// Usage documentation for a subcommand
#[derive(Debug, Default)]
pub(crate) struct Subcommand {
    /// Name of the subcommand
    pub(crate) name: String,

    /// Syntax summary, without command_name
    pub(crate) syntax: String,

    /// One-line description of the subcommand.  Use an initial capital and no trailing period.
    pub(crate) summary: String,

    /// Multi-line description of the subcommand.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: String,
}

impl Subcommand {
    fn write_help<W: Write>(&self, mut w: W, summary: bool) -> Result<()> {
        if summary {
            writeln!(w, "  task {} - {}", self.name, self.summary)?;
        } else {
            write!(
                w,
                "  task {}\n{}\n",
                self.syntax,
                indent(self.description.trim(), "    ")
            )?;
        }
        Ok(())
    }
}

/// Usage documentation for a filter argument
#[derive(Debug, Default)]
pub(crate) struct Filter {
    /// Syntax summary
    pub(crate) syntax: String,

    /// One-line description of the filter.  Use all-caps words for placeholders.
    pub(crate) summary: String,

    /// Multi-line description of the filter.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: String,
}

impl Filter {
    fn write_help<W: Write>(&self, mut w: W, summary: bool) -> Result<()> {
        if summary {
            write!(w, "  {} - {}\n", self.syntax, self.summary)?;
        } else {
            write!(
                w,
                "  {}\n{}\n",
                self.syntax,
                indent(self.description.trim(), "    ")
            )?;
        }
        Ok(())
    }
}

/// Usage documentation for a modification argument
#[derive(Debug, Default)]
pub(crate) struct Modification {
    /// Syntax summary
    pub(crate) syntax: String,

    /// One-line description of the modification.  Use all-caps words for placeholders.
    pub(crate) summary: String,

    /// Multi-line description of the modification.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: String,
}

impl Modification {
    fn write_help<W: Write>(&self, mut w: W, summary: bool) -> Result<()> {
        if summary {
            write!(w, "  {} - {}\n", self.syntax, self.summary)?;
        } else {
            write!(
                w,
                "  {}\n{}\n",
                self.syntax,
                indent(self.description.trim(), "    ")
            )?;
        }
        Ok(())
    }
}

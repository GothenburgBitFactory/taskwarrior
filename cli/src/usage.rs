//! This module handles creation of CLI usage documents (--help, manpages, etc.) in
//! a way that puts the source of that documentation near its implementation.

use crate::argparse;
use std::io::{Result, Write};
use textwrap::indent;

/// A top-level structure containing usage/help information for the entire CLI.
#[derive(Debug, Default)]
pub(crate) struct Usage {
    pub(crate) subcommands: Vec<Subcommand>,
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
        if !summary {
            writeln!(w, "\nSee `task help` for more detail")?;
        }
        Ok(())
    }
}

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

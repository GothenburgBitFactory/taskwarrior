//! This module handles creation of CLI usage documents (--help, manpages, etc.) in
//! a way that puts the source of that documentation near its implementation.

use crate::argparse;
use crate::settings;
use anyhow::Result;
use std::io::Write;

#[cfg(feature = "usage-docs")]
use std::fmt::Write as FmtWrite;

/// A top-level structure containing usage/help information for the entire CLI.
#[derive(Debug, Default)]
pub struct Usage {
    pub(crate) subcommands: Vec<Subcommand>,
    pub(crate) filters: Vec<Filter>,
    pub(crate) modifications: Vec<Modification>,
    pub(crate) report_properties: Vec<ReportProperty>,
}

impl Usage {
    /// Get a new, completely-filled-out usage object
    pub fn new() -> Self {
        let mut rv = Self {
            ..Default::default()
        };

        argparse::get_usage(&mut rv);
        settings::get_usage(&mut rv);

        rv
    }

    /// Write this usage to the given output as a help string, writing a short version if `summary`
    /// is true.
    pub(crate) fn write_help<W: Write>(
        &self,
        mut w: W,
        command_name: &str,
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
            subcommand.write_help(&mut w, command_name, summary)?;
        }
        writeln!(w, "Filter Expressions:\n")?;
        writeln!(
            w,
            "{}",
            indented(
                "
                Where [filter] appears above, zero or more of the following arguments can be used
                to limit the tasks addressed by the subcommand.",
                ""
            )
        )?;
        for filter in self.filters.iter() {
            filter.write_help(&mut w, command_name, summary)?;
        }
        writeln!(w, "Modifications:\n")?;
        writeln!(
            w,
            "{}",
            indented(
                "
                Where [modification] appears above, zero or more of the following arguments can be
                used to modify the selected tasks.",
                ""
            )
        )?;
        for modification in self.modifications.iter() {
            modification.write_help(&mut w, command_name, summary)?;
        }
        if !summary {
            writeln!(w, "\nSee `{} help` for more detail", command_name)?;
        }
        Ok(())
    }

    #[cfg(feature = "usage-docs")]
    /// Substitute strings matching
    ///
    /// ```text
    /// <!-- INSERT GENERATED DOCUMENTATION - $type -->
    /// ```
    ///
    /// With the appropriate documentation.
    pub fn substitute_docs(&self, content: &str) -> Result<String> {
        // this is not efficient, but it doesn't need to be
        let lines = content.lines();
        let mut w = String::new();

        const DOC_HEADER_PREFIX: &str = "<!-- INSERT GENERATED DOCUMENTATION - ";
        const DOC_HEADER_SUFFIX: &str = " -->";

        for line in lines {
            if line.starts_with(DOC_HEADER_PREFIX) && line.ends_with(DOC_HEADER_SUFFIX) {
                let doc_type = &line[DOC_HEADER_PREFIX.len()..line.len() - DOC_HEADER_SUFFIX.len()];

                match doc_type {
                    "subcommands" => {
                        for subcommand in self.subcommands.iter() {
                            subcommand.write_markdown(&mut w)?;
                        }
                    }
                    "filters" => {
                        for filter in self.filters.iter() {
                            filter.write_markdown(&mut w)?;
                        }
                    }
                    "modifications" => {
                        for modification in self.modifications.iter() {
                            modification.write_markdown(&mut w)?;
                        }
                    }
                    "report-columns" => {
                        for prop in self.report_properties.iter() {
                            prop.write_column_markdown(&mut w)?;
                        }
                    }
                    "report-sort-by" => {
                        for prop in self.report_properties.iter() {
                            prop.write_sort_by_markdown(&mut w)?;
                        }
                    }
                    _ => anyhow::bail!("Unkonwn doc type {}", doc_type),
                }
            } else {
                writeln!(w, "{}", line)?;
            }
        }

        Ok(w)
    }
}

/// wrap an indented string
fn indented(string: &str, indent: &str) -> String {
    let termwidth = textwrap::termwidth();
    let words: Vec<&str> = string.split_whitespace().collect();
    let string = words.join(" ");
    textwrap::indent(
        textwrap::fill(string.trim(), termwidth - indent.len()).as_ref(),
        indent,
    )
}

/// Usage documentation for a subcommand
#[derive(Debug, Default)]
pub(crate) struct Subcommand {
    /// Name of the subcommand
    pub(crate) name: &'static str,

    /// Syntax summary, without command_name
    pub(crate) syntax: &'static str,

    /// One-line description of the subcommand.  Use an initial capital and no trailing period.
    pub(crate) summary: &'static str,

    /// Multi-line description of the subcommand.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: &'static str,
}

impl Subcommand {
    fn write_help<W: Write>(&self, mut w: W, command_name: &str, summary: bool) -> Result<()> {
        if summary {
            writeln!(w, "  {} {} - {}", command_name, self.name, self.summary)?;
        } else {
            writeln!(
                w,
                "  {} {}\n{}",
                command_name,
                self.syntax,
                indented(self.description, "    ")
            )?;
        }
        Ok(())
    }

    #[cfg(feature = "usage-docs")]
    fn write_markdown<W: FmtWrite>(&self, mut w: W) -> Result<()> {
        writeln!(w, "### `ta {}` - {}", self.name, self.summary)?;
        writeln!(w, "```shell\nta {}\n```", self.syntax)?;
        writeln!(w, "{}", indented(self.description, ""))?;
        writeln!(w)?;
        Ok(())
    }
}

/// Usage documentation for a filter argument
#[derive(Debug, Default)]
pub(crate) struct Filter {
    /// Syntax summary
    pub(crate) syntax: &'static str,

    /// One-line description of the filter.  Use all-caps words for placeholders.
    pub(crate) summary: &'static str,

    /// Multi-line description of the filter.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: &'static str,
}

impl Filter {
    fn write_help<W: Write>(&self, mut w: W, _: &str, summary: bool) -> Result<()> {
        if summary {
            writeln!(w, "  {} - {}", self.syntax, self.summary)?;
        } else {
            write!(
                w,
                "  {}\n{}\n",
                self.syntax,
                indented(self.description, "    ")
            )?;
        }
        Ok(())
    }

    #[cfg(feature = "usage-docs")]
    fn write_markdown<W: FmtWrite>(&self, mut w: W) -> Result<()> {
        writeln!(w, "* `{}` - {}", self.syntax, self.summary)?;
        writeln!(w)?;
        writeln!(w, "{}", indented(self.description, "  "))?;
        writeln!(w)?;
        Ok(())
    }
}

/// Usage documentation for a modification argument
#[derive(Debug, Default)]
pub(crate) struct Modification {
    /// Syntax summary
    pub(crate) syntax: &'static str,

    /// One-line description of the modification.  Use all-caps words for placeholders.
    pub(crate) summary: &'static str,

    /// Multi-line description of the modification.  It's OK for this to duplicate summary, as the
    /// two are not displayed together.
    pub(crate) description: &'static str,
}

impl Modification {
    fn write_help<W: Write>(&self, mut w: W, _: &str, summary: bool) -> Result<()> {
        if summary {
            writeln!(w, "  {} - {}", self.syntax, self.summary)?;
        } else {
            writeln!(
                w,
                "  {}\n{}",
                self.syntax,
                indented(self.description, "    ")
            )?;
        }
        Ok(())
    }

    #[cfg(feature = "usage-docs")]
    fn write_markdown<W: FmtWrite>(&self, mut w: W) -> Result<()> {
        writeln!(w, "* `{}` - {}", self.syntax, self.summary)?;
        writeln!(w)?;
        writeln!(w, "{}", indented(self.description, "  "))?;
        writeln!(w)?;
        Ok(())
    }
}

/// Usage documentation for a report property (which may be used for sorting, as a column, or
/// both).
#[derive(Debug, Default)]
pub(crate) struct ReportProperty {
    /// Name of the property
    pub(crate) name: &'static str,

    /// Usage description for sorting, if any
    pub(crate) as_sort_by: Option<&'static str>,

    /// Usage description as a column, if any
    pub(crate) as_column: Option<&'static str>,
}

impl ReportProperty {
    #[cfg(feature = "usage-docs")]
    fn write_sort_by_markdown<W: FmtWrite>(&self, mut w: W) -> Result<()> {
        if let Some(as_sort_by) = self.as_sort_by {
            writeln!(w, "* `{}`", self.name)?;
            writeln!(w)?;
            writeln!(w, "{}", indented(as_sort_by, "  "))?;
            writeln!(w)?;
        }
        Ok(())
    }

    #[cfg(feature = "usage-docs")]
    fn write_column_markdown<W: FmtWrite>(&self, mut w: W) -> Result<()> {
        if let Some(as_column) = self.as_column {
            writeln!(w, "* `{}`", self.name)?;
            writeln!(w)?;
            writeln!(w, "{}", indented(as_column, "  "))?;
            writeln!(w)?;
        }
        Ok(())
    }
}

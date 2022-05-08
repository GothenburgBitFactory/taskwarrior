//! Support for the CLI's configuration file, including default settings.
//!
//! Configuration is stored in a "parsed" format, meaning that any syntax errors will be caught on
//! startup and not just when those values are used.

mod report;
mod settings;
mod util;

pub(crate) use report::{get_usage, Column, Property, Report, Sort, SortBy};
pub(crate) use settings::Settings;

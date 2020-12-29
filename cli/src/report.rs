//! This module contains the data structures used to define reports.

use crate::argparse::Filter;

/// A report specifies a filter as well as a sort order and information about which
/// task attributes to display
#[derive(Clone, Debug, PartialEq, Default)]
pub(crate) struct Report {
    /// Columns to display in this report
    pub columns: Vec<Column>,
    /// Sort order for this report
    pub sort: Vec<Sort>,
    /// Filter selecting tasks for this report
    pub filter: Filter,
}

/// A column to display in a report
#[derive(Clone, Debug, PartialEq)]
pub(crate) struct Column {
    /// The label for this column
    pub label: String,

    /// The property to display
    pub property: Property,
}

/// Task property to display in a report
#[derive(Clone, Debug, PartialEq)]
#[allow(dead_code)]
pub(crate) enum Property {
    /// The task's ID, either working-set index or Uuid if no in the working set
    Id,

    /// The task's full UUID
    Uuid,

    /// Whether the task is active or not
    Active,

    /// The task's description
    Description,

    /// The task's tags
    Tags,
}

/// A sorting criterion for a sort operation.
#[derive(Clone, Debug, PartialEq)]
pub(crate) struct Sort {
    /// True if the sort should be "ascending" (a -> z, 0 -> 9, etc.)
    pub ascending: bool,

    /// The property to sort on
    pub sort_by: SortBy,
}

/// Task property to sort by
#[derive(Clone, Debug, PartialEq)]
#[allow(dead_code)]
pub(crate) enum SortBy {
    /// The task's ID, either working-set index or a UUID prefix; working
    /// set tasks sort before others.
    Id,

    /// The task's full UUID
    Uuid,

    /// The task's description
    Description,
}

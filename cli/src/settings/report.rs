//! This module contains the data structures used to define reports.

use crate::argparse::{Condition, Filter};
use crate::settings::util::table_with_keys;
use crate::usage::{self, Usage};
use anyhow::{anyhow, bail, Result};
use std::convert::{TryFrom, TryInto};

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
pub(crate) enum Property {
    // NOTE: when adding a property here, add it to get_usage, below, as well.
    /// The task's ID, either working-set index or Uuid if not in the working set
    Id,

    /// The task's full UUID
    Uuid,

    /// Whether the task is active or not
    Active,

    /// The task's description
    Description,

    /// The task's tags
    Tags,

    /// The task's wait date
    Wait,
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
pub(crate) enum SortBy {
    // NOTE: when adding a property here, add it to get_usage, below, as well.
    /// The task's ID, either working-set index or a UUID prefix; working
    /// set tasks sort before others.
    Id,

    /// The task's full UUID
    Uuid,

    /// The task's description
    Description,

    /// The task's wait date
    Wait,
}

// Conversions from settings::Settings.

impl TryFrom<toml::Value> for Report {
    type Error = anyhow::Error;

    fn try_from(cfg: toml::Value) -> Result<Report> {
        Report::try_from(&cfg)
    }
}

impl TryFrom<&toml::Value> for Report {
    type Error = anyhow::Error;

    /// Create a Report from a toml value.  This should be the `report.<report_name>` value.
    /// The error message begins with any additional path information, e.g., `.sort[1].sort_by:
    /// ..`.
    fn try_from(cfg: &toml::Value) -> Result<Report> {
        let keys = ["sort", "columns", "filter"];
        let table = table_with_keys(cfg, &keys).map_err(|e| anyhow!(": {}", e))?;

        let sort = match table.get("sort") {
            Some(v) => v
                .as_array()
                .ok_or_else(|| anyhow!(".sort: not an array"))?
                .iter()
                .enumerate()
                .map(|(i, v)| v.try_into().map_err(|e| anyhow!(".sort[{}]{}", i, e)))
                .collect::<Result<Vec<_>>>()?,
            None => vec![],
        };

        let columns = match table.get("columns") {
            Some(v) => v
                .as_array()
                .ok_or_else(|| anyhow!(".columns: not an array"))?
                .iter()
                .enumerate()
                .map(|(i, v)| v.try_into().map_err(|e| anyhow!(".columns[{}]{}", i, e)))
                .collect::<Result<Vec<_>>>()?,
            None => bail!(": `columns` property is required"),
        };

        let conditions = match table.get("filter") {
            Some(v) => v
                .as_array()
                .ok_or_else(|| anyhow!(".filter: not an array"))?
                .iter()
                .enumerate()
                .map(|(i, v)| {
                    v.as_str()
                        .ok_or_else(|| anyhow!(".filter[{}]: not a string", i))
                        .and_then(|s| Condition::parse_str(s))
                        .map_err(|e| anyhow!(".filter[{}]: {}", i, e))
                })
                .collect::<Result<Vec<_>>>()?,
            None => vec![],
        };

        Ok(Report {
            columns,
            sort,
            filter: Filter { conditions },
        })
    }
}

impl TryFrom<&toml::Value> for Column {
    type Error = anyhow::Error;

    fn try_from(cfg: &toml::Value) -> Result<Column> {
        let keys = ["label", "property"];
        let table = table_with_keys(cfg, &keys).map_err(|e| anyhow!(": {}", e))?;

        let label = match table.get("label") {
            Some(v) => v
                .as_str()
                .ok_or_else(|| anyhow!(".label: not a string"))?
                .to_owned(),
            None => bail!(": `label` property is required"),
        };

        let property = match table.get("property") {
            Some(v) => v.try_into().map_err(|e| anyhow!(".property{}", e))?,
            None => bail!(": `property` property is required"),
        };

        Ok(Column { label, property })
    }
}

impl TryFrom<&toml::Value> for Property {
    type Error = anyhow::Error;

    fn try_from(cfg: &toml::Value) -> Result<Property> {
        let s = cfg.as_str().ok_or_else(|| anyhow!(": not a string"))?;
        Ok(match s {
            "id" => Property::Id,
            "uuid" => Property::Uuid,
            "active" => Property::Active,
            "description" => Property::Description,
            "tags" => Property::Tags,
            "wait" => Property::Wait,
            _ => bail!(": unknown property {}", s),
        })
    }
}

impl TryFrom<&toml::Value> for Sort {
    type Error = anyhow::Error;

    fn try_from(cfg: &toml::Value) -> Result<Sort> {
        let keys = ["ascending", "sort_by"];
        let table = table_with_keys(cfg, &keys).map_err(|e| anyhow!(": {}", e))?;
        let ascending = match table.get("ascending") {
            Some(v) => v
                .as_bool()
                .ok_or_else(|| anyhow!(".ascending: not a boolean value"))?,
            None => true, // default
        };

        let sort_by = match table.get("sort_by") {
            Some(v) => v.try_into().map_err(|e| anyhow!(".sort_by{}", e))?,
            None => bail!(": `sort_by` property is required"),
        };

        Ok(Sort { ascending, sort_by })
    }
}

impl TryFrom<&toml::Value> for SortBy {
    type Error = anyhow::Error;

    fn try_from(cfg: &toml::Value) -> Result<SortBy> {
        let s = cfg.as_str().ok_or_else(|| anyhow!(": not a string"))?;
        Ok(match s {
            "id" => SortBy::Id,
            "uuid" => SortBy::Uuid,
            "description" => SortBy::Description,
            "wait" => SortBy::Wait,
            _ => bail!(": unknown sort_by value `{}`", s),
        })
    }
}

pub(crate) fn get_usage(u: &mut Usage) {
    u.report_properties.push(usage::ReportProperty {
        name: "id",
        as_sort_by: Some("Sort by the task's shorthand ID"),
        as_column: Some("The task's shorthand ID"),
    });
    u.report_properties.push(usage::ReportProperty {
        name: "uuid",
        as_sort_by: Some("Sort by the task's full UUID"),
        as_column: Some("The task's full UUID"),
    });
    u.report_properties.push(usage::ReportProperty {
        name: "active",
        as_sort_by: None,
        as_column: Some("`*` if the task is active (started)"),
    });
    u.report_properties.push(usage::ReportProperty {
        name: "wait",
        as_sort_by: Some("Sort by the task's wait date, with non-waiting tasks first"),
        as_column: Some("Wait date of the task"),
    });
    u.report_properties.push(usage::ReportProperty {
        name: "description",
        as_sort_by: Some("Sort by the task's description"),
        as_column: Some("The task's description"),
    });
    u.report_properties.push(usage::ReportProperty {
        name: "tags",
        as_sort_by: None,
        as_column: Some("The task's tags"),
    });
}

#[cfg(test)]
mod test {
    use super::*;
    use taskchampion::Status;
    use toml::toml;

    #[test]
    fn test_report_ok() {
        let val = toml! {
            sort = []
            columns = []
            filter = ["status:pending"]
        };
        let report: Report = TryInto::try_into(val).unwrap();
        assert_eq!(
            report.filter,
            Filter {
                conditions: vec![Condition::Status(Status::Pending),],
            }
        );
        assert_eq!(report.columns, vec![]);
        assert_eq!(report.sort, vec![]);
    }

    #[test]
    fn test_report_no_sort() {
        let val = toml! {
            filter = []
            columns = []
        };
        let report = Report::try_from(val).unwrap();
        assert_eq!(report.sort, vec![]);
    }

    #[test]
    fn test_report_sort_not_array() {
        let val = toml! {
            filter = []
            sort = true
            columns = []
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ".sort: not an array");
    }

    #[test]
    fn test_report_sort_error() {
        let val = toml! {
            filter = []
            sort = [ { sort_by = "id" }, true ]
            columns = []
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert!(err.starts_with(".sort[1]"));
    }

    #[test]
    fn test_report_unknown_prop() {
        let val = toml! {
            columns = []
            filter = []
            sort = []
            nosuch = true
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ": unknown table key `nosuch`");
    }

    #[test]
    fn test_report_no_columns() {
        let val = toml! {
            filter = []
            sort = []
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ": `columns` property is required");
    }

    #[test]
    fn test_report_columns_not_array() {
        let val = toml! {
            filter = []
            sort = []
            columns = true
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ".columns: not an array");
    }

    #[test]
    fn test_report_column_error() {
        let val = toml! {
            filter = []
            sort = []

            [[columns]]
            label = "ID"
            property = "id"

            [[columns]]
            foo = 10
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ".columns[1]: unknown table key `foo`");
    }

    #[test]
    fn test_report_filter_not_array() {
        let val = toml! {
            filter = "foo"
            sort = []
            columns = []
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert_eq!(&err, ".filter: not an array");
    }

    #[test]
    fn test_report_filter_error() {
        let val = toml! {
            sort = []
            columns = []
            filter = [ "nosuchfilter" ]
        };
        let err = Report::try_from(val).unwrap_err().to_string();
        assert!(err.starts_with(".filter[0]: invalid filter condition:"));
    }

    #[test]
    fn test_column() {
        let val = toml! {
            label = "ID"
            property = "id"
        };
        let column = Column::try_from(&val).unwrap();
        assert_eq!(
            column,
            Column {
                label: "ID".to_owned(),
                property: Property::Id,
            }
        );
    }

    #[test]
    fn test_column_unknown_prop() {
        let val = toml! {
            label = "ID"
            property = "id"
            nosuch = "foo"
        };
        assert_eq!(
            &Column::try_from(&val).unwrap_err().to_string(),
            ": unknown table key `nosuch`"
        );
    }

    #[test]
    fn test_column_no_label() {
        let val = toml! {
            property = "id"
        };
        assert_eq!(
            &Column::try_from(&val).unwrap_err().to_string(),
            ": `label` property is required"
        );
    }

    #[test]
    fn test_column_invalid_label() {
        let val = toml! {
            label = []
            property = "id"
        };
        assert_eq!(
            &Column::try_from(&val).unwrap_err().to_string(),
            ".label: not a string"
        );
    }

    #[test]
    fn test_column_no_property() {
        let val = toml! {
            label = "ID"
        };
        assert_eq!(
            &Column::try_from(&val).unwrap_err().to_string(),
            ": `property` property is required"
        );
    }

    #[test]
    fn test_column_invalid_property() {
        let val = toml! {
            label = "ID"
            property = []
        };
        assert_eq!(
            &Column::try_from(&val).unwrap_err().to_string(),
            ".property: not a string"
        );
    }

    #[test]
    fn test_property() {
        let val = toml::Value::String("uuid".to_owned());
        let prop = Property::try_from(&val).unwrap();
        assert_eq!(prop, Property::Uuid);
    }

    #[test]
    fn test_property_invalid_type() {
        let val = toml::Value::Array(vec![]);
        assert_eq!(
            &Property::try_from(&val).unwrap_err().to_string(),
            ": not a string"
        );
    }

    #[test]
    fn test_sort() {
        let val = toml! {
            ascending = false
            sort_by = "id"
        };
        let sort = Sort::try_from(&val).unwrap();
        assert_eq!(
            sort,
            Sort {
                ascending: false,
                sort_by: SortBy::Id,
            }
        );
    }

    #[test]
    fn test_sort_no_ascending() {
        let val = toml! {
            sort_by = "id"
        };
        let sort = Sort::try_from(&val).unwrap();
        assert_eq!(
            sort,
            Sort {
                ascending: true,
                sort_by: SortBy::Id,
            }
        );
    }

    #[test]
    fn test_sort_unknown_prop() {
        let val = toml! {
            sort_by = "id"
            nosuch = true
        };
        assert_eq!(
            &Sort::try_from(&val).unwrap_err().to_string(),
            ": unknown table key `nosuch`"
        );
    }

    #[test]
    fn test_sort_no_sort_by() {
        let val = toml! {
            ascending = true
        };
        assert_eq!(
            &Sort::try_from(&val).unwrap_err().to_string(),
            ": `sort_by` property is required"
        );
    }

    #[test]
    fn test_sort_invalid_ascending() {
        let val = toml! {
            sort_by = "id"
            ascending = {}
        };
        assert_eq!(
            &Sort::try_from(&val).unwrap_err().to_string(),
            ".ascending: not a boolean value"
        );
    }

    #[test]
    fn test_sort_invalid_sort_by() {
        let val = toml! {
            sort_by = {}
        };
        assert_eq!(
            &Sort::try_from(&val).unwrap_err().to_string(),
            ".sort_by: not a string"
        );
    }

    #[test]
    fn test_sort_by() {
        let val = toml::Value::String("uuid".to_string());
        let prop = SortBy::try_from(&val).unwrap();
        assert_eq!(prop, SortBy::Uuid);
    }

    #[test]
    fn test_sort_by_unknown() {
        let val = toml::Value::String("nosuch".to_string());
        assert_eq!(
            &SortBy::try_from(&val).unwrap_err().to_string(),
            ": unknown sort_by value `nosuch`"
        );
    }

    #[test]
    fn test_sort_by_invalid_type() {
        let val = toml::Value::Array(vec![]);
        assert_eq!(
            &SortBy::try_from(&val).unwrap_err().to_string(),
            ": not a string"
        );
    }
}

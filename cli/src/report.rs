//! This module contains the data structures used to define reports.

use crate::argparse::{Condition, Filter};
use anyhow::{bail};

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

// Conversions from config::Value.  Note that these cannot ergonomically use TryFrom/TryInto; see
// https://github.com/mehcode/config-rs/issues/162

impl Report {
    /// Create a Report from a config value.  This should be the `report.<report_name>` value.
    /// The error message begins with any additional path information, e.g., `.sort[1].sort_by:
    /// ..`.
    pub(crate) fn from_config(cfg: config::Value) -> anyhow::Result<Report> {
        let mut map = cfg.into_table().map_err(|e| anyhow::anyhow!(": {}", e))?;
        let sort = if let Some(sort_array) = map.remove("sort") {
            sort_array
                .into_array()
                .map_err(|e| anyhow::anyhow!(".sort: {}", e))?
                .drain(..)
                .enumerate()
                .map(|(i, v)| Sort::from_config(v).map_err(|e| anyhow::anyhow!(".sort[{}]{}", i, e)))
                .collect::<anyhow::Result<Vec<_>>>()?
        } else {
            vec![]
        };

        let columns = map
            .remove("columns")
            .ok_or_else(|| anyhow::anyhow!(": 'columns' property is required"))?
            .into_array()
            .map_err(|e| anyhow::anyhow!(".columns: {}", e))?
            .drain(..)
            .enumerate()
            .map(|(i, v)| Column::from_config(v).map_err(|e| anyhow::anyhow!(".columns[{}]{}", i, e)))
            .collect::<anyhow::Result<Vec<_>>>()?;

        let conditions = if let Some(conditions) = map.remove("filter") {
            conditions
                .into_array()
                .map_err(|e| anyhow::anyhow!(".filter: {}", e))?
                .drain(..)
                .enumerate()
                .map(|(i, v)| {
                    v.into_str()
                        .map_err(|e| e.into())
                        .and_then(|s| Condition::parse_str(&s))
                        .map_err(|e| anyhow::anyhow!(".filter[{}]: {}", i, e))
                })
                .collect::<anyhow::Result<Vec<_>>>()?
        } else {
            vec![]
        };

        let filter = Filter { conditions };

        if !map.is_empty() {
            bail!(": unknown properties");
        }

        Ok(Report {
            columns,
            sort,
            filter,
        })
    }
}

impl Column {
    pub(crate) fn from_config(cfg: config::Value) -> anyhow::Result<Column> {
        let mut map = cfg.into_table().map_err(|e| anyhow::anyhow!(": {}", e))?;
        let label = map
            .remove("label")
            .ok_or_else(|| anyhow::anyhow!(": 'label' property is required"))?
            .into_str()
            .map_err(|e| anyhow::anyhow!(".label: {}", e))?;
        let property: config::Value = map
            .remove("property")
            .ok_or_else(|| anyhow::anyhow!(": 'property' property is required"))?;
        let property =
            Property::from_config(property).map_err(|e| anyhow::anyhow!(".property{}", e))?;

        if !map.is_empty() {
            bail!(": unknown properties");
        }

        Ok(Column { label, property })
    }
}

impl Property {
    pub(crate) fn from_config(cfg: config::Value) -> anyhow::Result<Property> {
        let s = cfg.into_str().map_err(|e| anyhow::anyhow!(": {}", e))?;
        Ok(match s.as_ref() {
            "id" => Property::Id,
            "uuid" => Property::Uuid,
            "active" => Property::Active,
            "description" => Property::Description,
            "tags" => Property::Tags,
            _ => bail!(": unknown property {}", s),
        })
    }
}

impl Sort {
    pub(crate) fn from_config(cfg: config::Value) -> anyhow::Result<Sort> {
        let mut map = cfg.into_table().map_err(|e| anyhow::anyhow!(": {}", e))?;
        let ascending = match map.remove("ascending") {
            Some(v) => v
                .into_bool()
                .map_err(|e| anyhow::anyhow!(".ascending: {}", e))?,
            None => true, // default
        };
        let sort_by: config::Value = map
            .remove("sort_by")
            .ok_or_else(|| anyhow::anyhow!(": 'sort_by' property is required"))?;
        let sort_by = SortBy::from_config(sort_by).map_err(|e| anyhow::anyhow!(".sort_by{}", e))?;

        if !map.is_empty() {
            bail!(": unknown properties");
        }

        Ok(Sort { ascending, sort_by })
    }
}

impl SortBy {
    pub(crate) fn from_config(cfg: config::Value) -> anyhow::Result<SortBy> {
        let s = cfg.into_str().map_err(|e| anyhow::anyhow!(": {}", e))?;
        Ok(match s.as_ref() {
            "id" => SortBy::Id,
            "uuid" => SortBy::Uuid,
            "description" => SortBy::Description,
            _ => bail!(": unknown sort_by {}", s),
        })
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use config::{Config, File, FileFormat, FileSourceString};
    use taskchampion::Status;
    use textwrap::{dedent, indent};

    fn config_from(cfg: &str) -> config::Value {
        // wrap this in a "table" so that we can get any type of value at the top level.
        let yaml = format!("val:\n{}", indent(&dedent(&cfg), "    "));
        let mut settings = Config::new();
        let cfg_file: File<FileSourceString> = File::from_str(&yaml, FileFormat::Yaml);
        settings.merge(cfg_file).unwrap();
        settings.cache.into_table().unwrap().remove("val").unwrap()
    }

    #[test]
    fn test_report_ok() {
        let val = config_from(
            "
            filter: []
            sort: []
            columns: []
            filter:
             - status:pending",
        );
        let report = Report::from_config(val).unwrap();
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
        let val = config_from(
            "
            filter: []
            columns: []",
        );
        let report = Report::from_config(val).unwrap();
        assert_eq!(report.sort, vec![]);
    }

    #[test]
    fn test_report_sort_not_array() {
        let val = config_from(
            "
            filter: []
            sort: true
            columns: []",
        );
        assert_eq!(
            &Report::from_config(val).unwrap_err().to_string(),
            ".sort: invalid type: boolean `true`, expected an array"
        );
    }

    #[test]
    fn test_report_sort_error() {
        let val = config_from(
            "
            filter: []
            sort:
              - sort_by: id
              - true
            columns: []",
        );
        assert!(&Report::from_config(val)
            .unwrap_err()
            .to_string()
            .starts_with(".sort[1]"));
    }

    #[test]
    fn test_report_unknown_prop() {
        let val = config_from(
            "
            columns: []
            filter: []
            sort: []
            nosuch: true
            ",
        );
        assert_eq!(
            &Report::from_config(val).unwrap_err().to_string(),
            ": unknown properties"
        );
    }

    #[test]
    fn test_report_no_columns() {
        let val = config_from(
            "
            filter: []
            sort: []",
        );
        assert_eq!(
            &Report::from_config(val).unwrap_err().to_string(),
            ": \'columns\' property is required"
        );
    }

    #[test]
    fn test_report_columns_not_array() {
        let val = config_from(
            "
            filter: []
            sort: []
            columns: true",
        );
        assert_eq!(
            &Report::from_config(val).unwrap_err().to_string(),
            ".columns: invalid type: boolean `true`, expected an array"
        );
    }

    #[test]
    fn test_report_column_error() {
        let val = config_from(
            "
            filter: []
            sort: []
            columns:
              - label: ID
                property: id
              - true",
        );
        assert!(&Report::from_config(val)
            .unwrap_err()
            .to_string()
            .starts_with(".columns[1]:"));
    }

    #[test]
    fn test_report_filter_not_array() {
        let val = config_from(
            "
            filter: []
            sort: []
            columns: []
            filter: true",
        );
        assert_eq!(
            &Report::from_config(val).unwrap_err().to_string(),
            ".filter: invalid type: boolean `true`, expected an array"
        );
    }

    #[test]
    fn test_report_filter_error() {
        let val = config_from(
            "
            filter: []
            sort: []
            columns: []
            filter:
              - nosuchfilter",
        );
        assert!(&Report::from_config(val)
            .unwrap_err()
            .to_string()
            .starts_with(".filter[0]: invalid filter condition:"));
    }

    #[test]
    fn test_column() {
        let val = config_from(
            "
            label: ID
            property: id",
        );
        let column = Column::from_config(val).unwrap();
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
        let val = config_from(
            "
            label: ID
            property: id
            nosuch: foo",
        );
        assert_eq!(
            &Column::from_config(val).unwrap_err().to_string(),
            ": unknown properties"
        );
    }

    #[test]
    fn test_column_no_label() {
        let val = config_from(
            "
            property: id",
        );
        assert_eq!(
            &Column::from_config(val).unwrap_err().to_string(),
            ": 'label' property is required"
        );
    }

    #[test]
    fn test_column_invalid_label() {
        let val = config_from(
            "
            label: []
            property: id",
        );
        assert_eq!(
            &Column::from_config(val).unwrap_err().to_string(),
            ".label: invalid type: sequence, expected a string"
        );
    }

    #[test]
    fn test_column_no_property() {
        let val = config_from(
            "
            label: ID",
        );
        assert_eq!(
            &Column::from_config(val).unwrap_err().to_string(),
            ": 'property' property is required"
        );
    }

    #[test]
    fn test_column_invalid_property() {
        let val = config_from(
            "
            label: ID
            property: []",
        );
        assert_eq!(
            &Column::from_config(val).unwrap_err().to_string(),
            ".property: invalid type: sequence, expected a string"
        );
    }

    #[test]
    fn test_property() {
        let val = config_from("uuid");
        let prop = Property::from_config(val).unwrap();
        assert_eq!(prop, Property::Uuid);
    }

    #[test]
    fn test_property_invalid_type() {
        let val = config_from("{}");
        assert_eq!(
            &Property::from_config(val).unwrap_err().to_string(),
            ": invalid type: map, expected a string"
        );
    }

    #[test]
    fn test_sort() {
        let val = config_from(
            "
            ascending: false
            sort_by: id",
        );
        let sort = Sort::from_config(val).unwrap();
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
        let val = config_from(
            "
            sort_by: id",
        );
        let sort = Sort::from_config(val).unwrap();
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
        let val = config_from(
            "
            sort_by: id
            nosuch: foo",
        );
        assert_eq!(
            &Sort::from_config(val).unwrap_err().to_string(),
            ": unknown properties"
        );
    }

    #[test]
    fn test_sort_no_sort_by() {
        let val = config_from(
            "
            ascending: true",
        );
        assert_eq!(
            &Sort::from_config(val).unwrap_err().to_string(),
            ": 'sort_by' property is required"
        );
    }

    #[test]
    fn test_sort_invalid_ascending() {
        let val = config_from(
            "
            sort_by: id
            ascending: {}",
        );
        assert_eq!(
            &Sort::from_config(val).unwrap_err().to_string(),
            ".ascending: invalid type: map, expected a boolean"
        );
    }

    #[test]
    fn test_sort_invalid_sort_by() {
        let val = config_from(
            "
            sort_by: {}",
        );
        assert_eq!(
            &Sort::from_config(val).unwrap_err().to_string(),
            ".sort_by: invalid type: map, expected a string"
        );
    }

    #[test]
    fn test_sort_by() {
        let val = config_from("uuid");
        let prop = SortBy::from_config(val).unwrap();
        assert_eq!(prop, SortBy::Uuid);
    }

    #[test]
    fn test_sort_by_unknown() {
        let val = config_from("nosuch");
        assert_eq!(
            &SortBy::from_config(val).unwrap_err().to_string(),
            ": unknown sort_by nosuch"
        );
    }

    #[test]
    fn test_sort_by_invalid_type() {
        let val = config_from("{}");
        assert_eq!(
            &SortBy::from_config(val).unwrap_err().to_string(),
            ": invalid type: map, expected a string"
        );
    }
}

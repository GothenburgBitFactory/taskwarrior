//! Support for the CLI's configuration file, including default settings.
//!
//! Configuration is stored in a "parsed" format, meaning that any syntax errors will be caught on
//! startup and not just when those values are used.

mod report;
mod util;

use crate::argparse::{Condition, Filter};
use anyhow::{anyhow, Context, Result};
use std::collections::HashMap;
use std::convert::TryFrom;
use std::env;
use std::fs;
use std::path::PathBuf;
use taskchampion::Status;
use toml::value::Table;
use util::table_with_keys;

pub(crate) use report::{Column, Property, Report, Sort, SortBy};

#[derive(Debug)]
pub(crate) struct Settings {
    // replica
    pub(crate) data_dir: PathBuf,

    // remote sync server
    pub(crate) server_client_key: Option<String>,
    pub(crate) server_origin: Option<String>,
    pub(crate) encryption_secret: Option<String>,

    // local sync server
    pub(crate) server_dir: PathBuf,

    // reports
    pub(crate) reports: HashMap<String, Report>,
}

impl Settings {
    pub(crate) fn read() -> Result<Self> {
        if let Some(config_file) = env::var_os("TASKCHAMPION_CONFIG") {
            log::debug!("Loading configuration from {:?}", config_file);
            env::remove_var("TASKCHAMPION_CONFIG");
            Self::load_from_file(config_file.into(), true)
        } else if let Some(mut dir) = dirs_next::config_dir() {
            dir.push("taskchampion.toml");
            log::debug!("Loading configuration from {:?} (optional)", dir);
            Self::load_from_file(dir, false)
        } else {
            Ok(Default::default())
        }
    }

    fn load_from_file(config_file: PathBuf, required: bool) -> Result<Self> {
        let mut settings = Self::default();

        let config_toml = match fs::read_to_string(config_file.clone()) {
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                return if required {
                    Err(e.into())
                } else {
                    Ok(settings)
                };
            }
            Err(e) => return Err(e.into()),
            Ok(s) => s,
        };

        let config_toml = config_toml
            .parse::<toml::Value>()
            .with_context(|| format!("error while reading {:?}", config_file))?;
        settings
            .update_from_toml(&config_toml)
            .with_context(|| format!("error while parsing {:?}", config_file))?;

        Ok(settings)
    }

    /// Update this object with configuration from the given config file.  This is
    /// broken out mostly for convenience in error handling
    fn update_from_toml(&mut self, config_toml: &toml::Value) -> Result<()> {
        let table_keys = [
            "data_dir",
            "server_client_key",
            "server_origin",
            "encryption_secret",
            "server_dir",
            "reports",
        ];
        let table = table_with_keys(&config_toml, &table_keys)?;

        fn get_str_cfg<F: FnOnce(String)>(
            table: &Table,
            name: &'static str,
            setter: F,
        ) -> Result<()> {
            if let Some(v) = table.get(name) {
                setter(
                    v.as_str()
                        .ok_or_else(|| anyhow!(".{}: not a string", name))?
                        .to_owned(),
                );
            }
            Ok(())
        }

        get_str_cfg(table, "data_dir", |v| {
            self.data_dir = v.into();
        })?;

        get_str_cfg(table, "server_client_key", |v| {
            self.server_client_key = Some(v);
        })?;

        get_str_cfg(table, "server_origin", |v| {
            self.server_origin = Some(v);
        })?;

        get_str_cfg(table, "encryption_secret", |v| {
            self.encryption_secret = Some(v);
        })?;

        get_str_cfg(table, "server_dir", |v| {
            self.server_dir = v.into();
        })?;

        if let Some(v) = table.get("reports") {
            let report_cfgs = v
                .as_table()
                .ok_or_else(|| anyhow!(".reports: not a table"))?;
            for (name, cfg) in report_cfgs {
                let report = Report::try_from(cfg).map_err(|e| anyhow!("reports.{}{}", name, e))?;
                self.reports.insert(name.clone(), report);
            }
        }

        Ok(())
    }
}

impl Default for Settings {
    fn default() -> Self {
        let data_dir;
        let server_dir;

        if let Some(dir) = dirs_next::data_local_dir() {
            data_dir = dir.join("taskchampion");
            server_dir = dir.join("taskchampion-sync-server");
        } else {
            // fallback
            data_dir = PathBuf::from(".");
            server_dir = PathBuf::from(".");
        }

        // define the default reports
        let mut reports = HashMap::new();

        reports.insert(
            "list".to_owned(),
            Report {
                sort: vec![Sort {
                    ascending: true,
                    sort_by: SortBy::Uuid,
                }],
                columns: vec![
                    Column {
                        label: "id".to_owned(),
                        property: Property::Id,
                    },
                    Column {
                        label: "description".to_owned(),
                        property: Property::Description,
                    },
                    Column {
                        label: "active".to_owned(),
                        property: Property::Active,
                    },
                    Column {
                        label: "tags".to_owned(),
                        property: Property::Tags,
                    },
                ],
                filter: Default::default(),
            },
        );

        reports.insert(
            "next".to_owned(),
            Report {
                sort: vec![
                    Sort {
                        ascending: true,
                        sort_by: SortBy::Id,
                    },
                    Sort {
                        ascending: true,
                        sort_by: SortBy::Uuid,
                    },
                ],
                columns: vec![
                    Column {
                        label: "id".to_owned(),
                        property: Property::Id,
                    },
                    Column {
                        label: "description".to_owned(),
                        property: Property::Description,
                    },
                    Column {
                        label: "active".to_owned(),
                        property: Property::Active,
                    },
                    Column {
                        label: "tags".to_owned(),
                        property: Property::Tags,
                    },
                ],
                filter: Filter {
                    conditions: vec![Condition::Status(Status::Pending)],
                },
            },
        );

        Self {
            data_dir,
            server_client_key: None,
            server_origin: None,
            encryption_secret: None,
            server_dir,
            reports,
        }
    }
}

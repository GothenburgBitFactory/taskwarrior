use super::util::table_with_keys;
use super::{Column, Property, Report, Sort, SortBy};
use crate::argparse::{Condition, Filter};
use anyhow::{anyhow, bail, Context, Result};
use std::collections::HashMap;
use std::convert::TryFrom;
use std::env;
use std::fs;
use std::path::PathBuf;
use taskchampion::Status;
use toml::value::Table;
use toml_edit::Document;

#[derive(Debug, PartialEq)]
pub(crate) struct Settings {
    /// filename from which this configuration was loaded, if any
    pub(crate) filename: Option<PathBuf>,

    /// Maximum number of tasks to modify without a confirmation prompt; `Some(0)` means to never
    /// prompt, and `None` means to use the default value.
    pub(crate) modification_count_prompt: Option<i64>,

    /// replica
    pub(crate) data_dir: PathBuf,

    /// remote sync server
    pub(crate) server_client_key: Option<String>,
    pub(crate) server_origin: Option<String>,
    pub(crate) encryption_secret: Option<String>,

    /// local sync server
    pub(crate) server_dir: PathBuf,

    /// reports
    pub(crate) reports: HashMap<String, Report>,
}

impl Settings {
    pub(crate) fn read() -> Result<Self> {
        if let Some(config_file) = env::var_os("TASKCHAMPION_CONFIG") {
            log::debug!("Loading configuration from {:?}", config_file);
            env::remove_var("TASKCHAMPION_CONFIG");
            Self::load_from_file(config_file.into(), true)
        } else if let Some(filename) = Settings::default_filename() {
            log::debug!("Loading configuration from {:?} (optional)", filename);
            Self::load_from_file(filename, false)
        } else {
            Ok(Default::default())
        }
    }

    /// Get the default filename for the configuration, or None if that cannot
    /// be determined.
    fn default_filename() -> Option<PathBuf> {
        dirs_next::config_dir().map(|dir| dir.join("taskchampion.toml"))
    }

    /// Update this settings object with the contents of the given TOML file.  Top-level settings
    /// are overwritten, and reports are overwritten by name.
    pub(crate) fn load_from_file(config_file: PathBuf, required: bool) -> Result<Self> {
        let mut settings = Self::default();

        let config_toml = match fs::read_to_string(config_file.clone()) {
            Err(e) if e.kind() == std::io::ErrorKind::NotFound => {
                return if required {
                    Err(e.into())
                } else {
                    settings.filename = Some(config_file);
                    Ok(settings)
                };
            }
            Err(e) => return Err(e.into()),
            Ok(s) => s,
        };

        let config_toml = config_toml
            .parse::<toml::Value>()
            .with_context(|| format!("error while reading {:?}", config_file))?;

        settings.filename = Some(config_file.clone());
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
            "modification_count_prompt",
            "server_client_key",
            "server_origin",
            "encryption_secret",
            "server_dir",
            "reports",
        ];
        let table = table_with_keys(config_toml, &table_keys)?;

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

        fn get_i64_cfg<F: FnOnce(i64)>(table: &Table, name: &'static str, setter: F) -> Result<()> {
            if let Some(v) = table.get(name) {
                setter(
                    v.as_integer()
                        .ok_or_else(|| anyhow!(".{}: not a number", name))?,
                );
            }
            Ok(())
        }

        get_str_cfg(table, "data_dir", |v| {
            self.data_dir = v.into();
        })?;

        get_i64_cfg(table, "modification_count_prompt", |v| {
            self.modification_count_prompt = Some(v);
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

    /// Set a value in the config file, modifying it in place.  Returns the filename.  The value is
    /// interpreted as the appropriate type for the configuration setting.
    pub(crate) fn set(&self, key: &str, value: &str) -> Result<PathBuf> {
        let allowed_keys = [
            "data_dir",
            "modification_count_prompt",
            "server_client_key",
            "server_origin",
            "encryption_secret",
            "server_dir",
            // reports is not allowed, since it is not a string
        ];
        if !allowed_keys.contains(&key) {
            bail!("No such configuration key {}", key);
        }

        let filename = if let Some(ref f) = self.filename {
            f.clone()
        } else {
            Settings::default_filename()
                .ok_or_else(|| anyhow!("Could not determine config file name"))?
        };

        let exists = filename.exists();

        // try to create the parent directory if the file does not exist
        if !exists {
            if let Some(dir) = filename.parent() {
                fs::create_dir_all(dir)?;
            }
        }

        // start with the existing document, or a blank document
        let mut document = if exists {
            fs::read_to_string(filename.clone())
                .context("Could not read existing configuration file")?
                .parse::<Document>()
                .context("Could not parse existing configuration file")?
        } else {
            Document::new()
        };

        // set the value as the correct type
        match key {
            // integers
            "modification_count_prompt" => {
                let value: i64 = value.parse()?;
                document[key] = toml_edit::value(value);
            }

            // most keys are strings
            _ => document[key] = toml_edit::value(value),
        }

        fs::write(filename.clone(), document.to_string())
            .context("Could not write updated configuration file")?;

        Ok(filename)
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
                    Column {
                        label: "wait".to_owned(),
                        property: Property::Wait,
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
            filename: None,
            data_dir,
            modification_count_prompt: None,
            server_client_key: None,
            server_origin: None,
            encryption_secret: None,
            server_dir,
            reports,
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;
    use tempfile::TempDir;
    use toml::toml;

    #[test]
    fn test_load_from_file_not_required() {
        let cfg_dir = TempDir::new().unwrap();
        let cfg_file = cfg_dir.path().join("foo.toml");

        let settings = Settings::load_from_file(cfg_file.clone(), false).unwrap();

        let mut expected = Settings::default();
        expected.filename = Some(cfg_file.clone());
        assert_eq!(settings, expected);
    }

    #[test]
    fn test_load_from_file_required() {
        let cfg_dir = TempDir::new().unwrap();

        assert!(Settings::load_from_file(cfg_dir.path().join("foo.toml"), true).is_err());
    }

    #[test]
    fn test_load_from_file_exists() {
        let cfg_dir = TempDir::new().unwrap();
        let cfg_file = cfg_dir.path().join("foo.toml");
        fs::write(cfg_file.clone(), "data_dir = \"/nowhere\"").unwrap();

        let settings = Settings::load_from_file(cfg_file.clone(), true).unwrap();
        assert_eq!(settings.data_dir, PathBuf::from("/nowhere"));
        assert_eq!(settings.filename, Some(cfg_file));
    }

    #[test]
    fn test_update_from_toml_top_level_keys() {
        let val = toml! {
            data_dir = "/data"
            modification_count_prompt = 42
            server_client_key = "sck"
            server_origin = "so"
            encryption_secret = "es"
            server_dir = "/server"
        };
        let mut settings = Settings::default();
        settings.update_from_toml(&val).unwrap();

        assert_eq!(settings.data_dir, PathBuf::from("/data"));
        assert_eq!(settings.modification_count_prompt, Some(42));
        assert_eq!(settings.server_client_key, Some("sck".to_owned()));
        assert_eq!(settings.server_origin, Some("so".to_owned()));
        assert_eq!(settings.encryption_secret, Some("es".to_owned()));
        assert_eq!(settings.server_dir, PathBuf::from("/server"));
    }

    #[test]
    fn test_update_from_toml_report() {
        let val = toml! {
            [reports.foo]
            sort = [ { sort_by = "id" } ]
            columns = [ { label = "ID", property = "id" } ]
        };
        let mut settings = Settings::default();
        settings.update_from_toml(&val).unwrap();

        assert!(settings.reports.get("foo").is_some());
        // beyond existence of this report, we can rely on Report's unit tests
    }

    #[test]
    fn test_set_valid_key() {
        let cfg_dir = TempDir::new().unwrap();
        let cfg_file = cfg_dir.path().join("foo.toml");
        fs::write(cfg_file.clone(), "server_dir = \"/srv\"").unwrap();

        let settings = Settings::load_from_file(cfg_file.clone(), true).unwrap();
        assert_eq!(settings.filename, Some(cfg_file.clone()));
        settings.set("data_dir", "/data").unwrap();
        settings.set("modification_count_prompt", "42").unwrap();

        // load the file again and see the changes
        let settings = Settings::load_from_file(cfg_file.clone(), true).unwrap();
        assert_eq!(settings.data_dir, PathBuf::from("/data"));
        assert_eq!(settings.server_dir, PathBuf::from("/srv"));
        assert_eq!(settings.filename, Some(cfg_file));
        assert_eq!(settings.modification_count_prompt, Some(42));
    }

    #[test]
    fn test_set_invalid_key() {
        let cfg_dir = TempDir::new().unwrap();
        let cfg_file = cfg_dir.path().join("foo.toml");
        fs::write(cfg_file.clone(), "server_dir = \"/srv\"").unwrap();

        let settings = Settings::load_from_file(cfg_file.clone(), true).unwrap();
        assert_eq!(settings.filename, Some(cfg_file.clone()));
        assert!(settings
            .set("modification_count_prompt", "a string?")
            .is_err());
    }
}

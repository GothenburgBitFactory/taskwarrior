use config::{Config, Environment, File, FileFormat, FileSourceFile, FileSourceString};
use failure::Fallible;
use std::env;
use std::path::PathBuf;

const DEFAULTS: &str = r#"
reports:
    list:
        sort:
          - sort_by: uuid
        columns:
          - label: Id
            property: id
          - label: Description
            property: description
          - label: Active
            property: active
          - label: Tags
            property: tags
    next:
        filter:
          - "status:pending"
        sort:
          - sort_by: uuid
        columns:
          - label: Id
            property: id
          - label: Description
            property: description
          - label: Active
            property: active
          - label: Tags
            property: tags
"#;

/// Get the default settings for this application
pub(crate) fn default_settings() -> Fallible<Config> {
    let mut settings = Config::default();

    // set up defaults
    if let Some(dir) = dirs::data_local_dir() {
        let mut tc_dir = dir.clone();
        tc_dir.push("taskchampion");
        settings.set_default(
            "data_dir",
            // the config crate does not support non-string paths
            tc_dir.to_str().expect("data_local_dir is not utf-8"),
        )?;

        let mut server_dir = dir;
        server_dir.push("taskchampion-sync-server");
        settings.set_default(
            "server_dir",
            // the config crate does not support non-string paths
            server_dir.to_str().expect("data_local_dir is not utf-8"),
        )?;
    }

    let defaults: File<FileSourceString> = File::from_str(DEFAULTS, FileFormat::Yaml);
    settings.merge(defaults)?;

    Ok(settings)
}

pub(crate) fn read_settings() -> Fallible<Config> {
    let mut settings = default_settings()?;

    // load either from the path in TASKCHAMPION_CONFIG, or from CONFIG_DIR/taskchampion
    if let Some(config_file) = env::var_os("TASKCHAMPION_CONFIG") {
        log::debug!("Loading configuration from {:?}", config_file);
        let config_file: PathBuf = config_file.into();
        let config_file: File<FileSourceFile> = config_file.into();
        settings.merge(config_file.required(true))?;
        env::remove_var("TASKCHAMPION_CONFIG");
    } else if let Some(mut dir) = dirs::config_dir() {
        dir.push("taskchampion");
        log::debug!("Loading configuration from {:?} (optional)", dir);
        let config_file: File<FileSourceFile> = dir.into();
        settings.merge(config_file.required(false))?;
    }

    // merge environment variables
    settings.merge(Environment::with_prefix("TASKCHAMPION"))?;

    Ok(settings)
}

use config::{Config, Environment, File, FileSourceFile};
use failure::Fallible;
use std::env;
use std::path::PathBuf;

pub(crate) fn read_settings() -> Fallible<Config> {
    let mut settings = Config::default();

    // set up defaults
    if let Some(mut dir) = dirs::data_local_dir() {
        dir.push("taskchampion");
        settings.set_default(
            "data_dir",
            // the config crate does not support non-string paths
            dir.to_str().expect("data_local_dir is not utf-8"),
        )?;
    }

    // load either from the path in TASKCHAMPION_CONFIG, or from CONFIG_DIR/taskchampion
    if let Some(config_file) = env::var_os("TASKCHAMPION_CONFIG") {
        let config_file: PathBuf = config_file.into();
        let config_file: File<FileSourceFile> = config_file.into();
        settings.merge(config_file.required(true))?;
        env::remove_var("TASKCHAMPION_CONFIG");
    } else if let Some(mut dir) = dirs::config_dir() {
        dir.push("taskchampion");
        let config_file: File<FileSourceFile> = dir.into();
        settings.merge(config_file.required(false))?;
    }

    // merge environment variables
    settings.merge(Environment::with_prefix("TASKCHAMPION"))?;

    Ok(settings)
}

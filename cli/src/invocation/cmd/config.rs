use crate::argparse::ConfigOperation;
use crate::settings::Settings;
use termcolor::{ColorSpec, WriteColor};

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    config_operation: ConfigOperation,
    settings: &Settings,
) -> Result<(), crate::Error> {
    match config_operation {
        ConfigOperation::Set(key, value) => {
            let filename = settings.set(&key, &value)?;
            write!(w, "Set configuration value ")?;
            w.set_color(ColorSpec::new().set_bold(true))?;
            write!(w, "{}", &key)?;
            w.set_color(ColorSpec::new().set_bold(false))?;
            write!(w, " in ")?;
            w.set_color(ColorSpec::new().set_bold(true))?;
            writeln!(w, "{:?}.", filename)?;
            w.set_color(ColorSpec::new().set_bold(false))?;
        }
        ConfigOperation::Path => {
            if let Some(ref filename) = settings.filename {
                writeln!(w, "{}", filename.to_string_lossy())?;
            } else {
                return Err(anyhow::anyhow!("No configuration filename found").into());
            }
        }
    }
    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use pretty_assertions::assert_eq;
    use std::fs;
    use tempfile::TempDir;

    #[test]
    fn test_config_set() {
        let cfg_dir = TempDir::new().unwrap();
        let cfg_file = cfg_dir.path().join("foo.toml");
        fs::write(
            cfg_file.clone(),
            "# store data everywhere\ndata_dir = \"/nowhere\"\n",
        )
        .unwrap();

        let settings = Settings::load_from_file(cfg_file.clone(), true).unwrap();

        let mut w = test_writer();

        execute(
            &mut w,
            ConfigOperation::Set("data_dir".to_owned(), "/somewhere".to_owned()),
            &settings,
        )
        .unwrap();
        assert!(w.into_string().starts_with("Set configuration value "));

        let updated_toml = fs::read_to_string(cfg_file.clone()).unwrap();
        dbg!(&updated_toml);
        assert_eq!(
            updated_toml,
            "# store data everywhere\ndata_dir = \"/somewhere\"\n"
        );
    }
}

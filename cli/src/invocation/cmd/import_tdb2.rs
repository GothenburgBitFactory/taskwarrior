use crate::tdb2;
use anyhow::anyhow;
use std::fs;
use std::path::PathBuf;
use taskchampion::{Replica, Uuid};
use termcolor::{Color, ColorSpec, WriteColor};

pub(crate) fn execute<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    path: &str,
) -> Result<(), crate::Error> {
    let path: PathBuf = path.into();

    let mut count = 0;
    for file in &["pending.data", "completed.data"] {
        let file = path.join(file);
        w.set_color(ColorSpec::new().set_bold(true))?;
        writeln!(w, "Importing tasks from {:?}.", file)?;
        w.reset()?;

        let data = fs::read_to_string(file)?;
        let content =
            tdb2::File::from_str(&data).map_err(|_| anyhow!("Could not parse TDB2 file format"))?;
        count += content.lines.len();
        for line in content.lines {
            import_task(w, replica, line)?;
        }
    }
    w.set_color(ColorSpec::new().set_bold(true))?;
    writeln!(w, "{} tasks imported.", count)?;
    w.reset()?;

    Ok(())
}

fn import_task<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    mut line: tdb2::Line,
) -> anyhow::Result<()> {
    let mut uuid = None;
    for attr in line.attrs.iter() {
        if &attr.name == "uuid" {
            uuid = Some(Uuid::parse_str(&attr.value)?);
            break;
        }
    }
    let uuid = uuid.ok_or_else(|| anyhow!("task has no uuid"))?;
    replica.create_task(uuid)?;

    let mut description = None;
    for attr in line.attrs.drain(..) {
        // oddly, TaskWarrior represents [ and ] with their HTML entity equivalents
        let value = attr.value.replace("&open;", "[").replace("&close;", "]");
        match attr.name.as_ref() {
            // `uuid` was already handled
            "uuid" => {}

            // everything else is inserted directly
            _ => {
                if attr.name == "description" {
                    // keep a copy of the description for console output
                    description = Some(value.clone());
                }
                replica.update_task(uuid, attr.name, Some(value))?;
            }
        }
    }

    w.set_color(ColorSpec::new().set_fg(Some(Color::Yellow)))?;
    write!(w, "{}", uuid)?;
    w.reset()?;
    writeln!(
        w,
        " {}",
        description.unwrap_or_else(|| "(no description)".into())
    )?;

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::invocation::test::*;
    use chrono::{TimeZone, Utc};
    use pretty_assertions::assert_eq;
    use std::convert::TryInto;
    use taskchampion::{Priority, Status};
    use tempfile::TempDir;

    #[test]
    fn test_import() -> anyhow::Result<()> {
        let mut w = test_writer();
        let mut replica = test_replica();
        let tmp_dir = TempDir::new()?;

        fs::write(
            tmp_dir.path().join("pending.data"),
            include_bytes!("pending.data"),
        )?;
        fs::write(
            tmp_dir.path().join("completed.data"),
            include_bytes!("completed.data"),
        )?;

        execute(&mut w, &mut replica, tmp_dir.path().to_str().unwrap())?;

        let task = replica
            .get_task(Uuid::parse_str("f19086c2-1f8d-4a6c-9b8d-f94901fb8e62").unwrap())
            .unwrap()
            .unwrap();
        assert_eq!(task.get_description(), "snake 🐍");
        assert_eq!(task.get_status(), Status::Pending);
        assert_eq!(task.get_priority(), Priority::M);
        assert_eq!(task.get_wait(), None);
        assert_eq!(
            task.get_modified(),
            Some(Utc.ymd(2022, 1, 8).and_hms(19, 33, 5))
        );
        assert!(task.has_tag(&"reptile".try_into().unwrap()));
        assert!(!task.has_tag(&"COMPLETED".try_into().unwrap()));

        let task = replica
            .get_task(Uuid::parse_str("4578fb67-359b-4483-afe4-fef15925ccd6").unwrap())
            .unwrap()
            .unwrap();
        assert_eq!(task.get_description(), "[TEST] foo");
        assert_eq!(task.get_status(), Status::Completed);
        assert_eq!(task.get_priority(), Priority::M);
        assert_eq!(task.get_wait(), None);
        assert_eq!(
            task.get_modified(),
            Some(Utc.ymd(2019, 3, 31).and_hms(23, 20, 16))
        );
        assert!(!task.has_tag(&"reptile".try_into().unwrap()));
        assert!(task.has_tag(&"COMPLETED".try_into().unwrap()));

        Ok(())
    }
}

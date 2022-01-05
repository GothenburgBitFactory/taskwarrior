use anyhow::{anyhow, bail};
use chrono::{DateTime, TimeZone, Utc};
use serde::{self, Deserialize, Deserializer};
use serde_json::Value;
use std::collections::HashMap;
use taskchampion::{Replica, Uuid};
use termcolor::WriteColor;

pub(crate) fn execute<W: WriteColor>(w: &mut W, replica: &mut Replica) -> Result<(), crate::Error> {
    writeln!(w, "Importing tasks from stdin.")?;
    let tasks: Vec<HashMap<String, Value>> =
        serde_json::from_reader(std::io::stdin()).map_err(|_| anyhow!("Invalid JSON"))?;

    for task_json in &tasks {
        import_task(w, replica, task_json)?;
    }

    writeln!(w, "{} tasks imported.", tasks.len())?;
    Ok(())
}

/// Convert the given value to a string, failing on compound types (arrays
/// and objects).
fn stringify(v: &Value) -> anyhow::Result<String> {
    Ok(match v {
        Value::String(ref s) => s.clone(),
        Value::Number(n) => n.to_string(),
        Value::Bool(true) => "true".to_string(),
        Value::Bool(false) => "false".to_string(),
        Value::Null => "null".to_string(),
        _ => bail!("{:?} cannot be converted to a string", v),
    })
}

pub fn deserialize_tw_datetime<'de, D>(deserializer: D) -> Result<DateTime<Utc>, D::Error>
where
    D: Deserializer<'de>,
{
    const FORMAT: &str = "%Y%m%dT%H%M%SZ";
    let s = String::deserialize(deserializer)?;
    Utc.datetime_from_str(&s, FORMAT)
        .map_err(serde::de::Error::custom)
}

/// Deserialize a string in the TaskWarrior format into a DateTime
#[derive(Deserialize)]
struct TwDateTime(#[serde(deserialize_with = "deserialize_tw_datetime")] DateTime<Utc>);

impl TwDateTime {
    /// Generate the data-model style UNIX timestamp for this DateTime
    fn tc_timestamp(&self) -> String {
        self.0.timestamp().to_string()
    }
}

#[derive(Deserialize)]
struct Annotation {
    entry: TwDateTime,
    description: String,
}

fn import_task<W: WriteColor>(
    w: &mut W,
    replica: &mut Replica,
    // TOOD: take this by value and consume it
    task_json: &HashMap<String, Value>,
) -> anyhow::Result<()> {
    let uuid = task_json
        .get("uuid")
        .ok_or_else(|| anyhow!("task has no uuid"))?;
    let uuid = uuid
        .as_str()
        .ok_or_else(|| anyhow!("uuid is not a string"))?;
    let uuid = Uuid::parse_str(uuid)?;
    replica.create_task(uuid)?;

    let mut description = None;
    for (k, v) in task_json.iter() {
        match k.as_ref() {
            // `id` is the working-set ID and is not stored
            "id" => {}

            // `urgency` is also calculated and not stored
            "urgency" => {}

            // `uuid` was already handled
            "uuid" => {}

            // `annotations` is a sub-aray
            "annotations" => {
                let annotations: Vec<Annotation> = serde_json::from_value(v.clone())?;
                for ann in annotations {
                    let k = format!("annotation_{}", ann.entry.tc_timestamp());
                    replica.update_task(uuid, k, Some(ann.description))?;
                }
            }

            // `depends` is a sub-aray
            "depends" => {
                let deps: Vec<String> = serde_json::from_value(v.clone())?;
                for dep in deps {
                    let k = format!("dep_{}", dep);
                    replica.update_task(uuid, k, Some("".to_owned()))?;
                }
            }

            // `tags` is a sub-aray
            "tags" => {
                let tags: Vec<String> = serde_json::from_value(v.clone())?;
                for tag in tags {
                    let k = format!("tag_{}", tag);
                    replica.update_task(uuid, k, Some("".to_owned()))?;
                }
            }

            // convert all datetimes -> epoch integers
            "end" | "entry" | "modified" | "wait" | "due" => {
                let v: TwDateTime = serde_json::from_value(v.clone())?;
                replica.update_task(uuid, k, Some(v.tc_timestamp()))?;
            }

            // everything else is inserted directly
            _ => {
                let v = stringify(v)?;
                replica.update_task(uuid, k, Some(v.clone()))?;
                if k == "description" {
                    description = Some(v);
                }
            }
        }
    }

    writeln!(
        w,
        "{} {}",
        uuid,
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
    use serde_json::json;
    use std::convert::TryInto;
    use taskchampion::{Priority, Status};

    #[test]
    fn stringify_string() {
        assert_eq!(stringify(&json!("foo")).unwrap(), "foo".to_string());
    }

    #[test]
    fn stringify_number() {
        assert_eq!(stringify(&json!(2.14)).unwrap(), "2.14".to_string());
    }

    #[test]
    fn stringify_bool() {
        assert_eq!(stringify(&json!(true)).unwrap(), "true".to_string());
        assert_eq!(stringify(&json!(false)).unwrap(), "false".to_string());
    }

    #[test]
    fn stringify_null() {
        assert_eq!(stringify(&json!(null)).unwrap(), "null".to_string());
    }

    #[test]
    fn stringify_invalid() {
        assert!(stringify(&json!([1])).is_err());
        assert!(stringify(&json!({"a": 1})).is_err());
    }

    #[test]
    fn test_import() -> anyhow::Result<()> {
        let mut w = test_writer();
        let mut replica = test_replica();

        let task_json = serde_json::from_value(json!({
          "id": 0,
          "description": "repair window",
          "end": "20211231T175614Z", // TODO (#327)
          "entry": "20211117T022410Z", // TODO (#326)
          "modified": "20211231T175614Z",
          "priority": "M",
          "status": "completed",
          "uuid": "fa01e916-1587-4c7d-a646-f7be62be8ee7",
          "wait": "20211225T001523Z",
          "due": "20211225T040000Z", // TODO (#82)

          // TODO: recurrence (#81)
          "imask": 2,
          "recur": "monthly",
          "rtype": "periodic",
          "mask": "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++--",

          // (legacy) UDAs
          "githubcreatedon": "20211110T175919Z",
          "githubnamespace": "djmitche",
          "githubnumber": 228,

          "tags": [
            "house"
          ],
          "depends": [ // TODO (#84)
            "4f71035d-1704-47f0-885c-6f9134bcefb2"
          ],
          "annotations": [
            {
              "entry": "20211223T142031Z",
              "description": "ordered from website"
            }
          ],
          "urgency": 4.16849
        }))?;
        import_task(&mut w, &mut replica, &task_json)?;

        let task = replica
            .get_task(Uuid::parse_str("fa01e916-1587-4c7d-a646-f7be62be8ee7").unwrap())
            .unwrap()
            .unwrap();
        assert_eq!(task.get_description(), "repair window");
        assert_eq!(task.get_status(), Status::Completed);
        assert_eq!(task.get_priority(), Priority::M);
        assert_eq!(
            task.get_wait(),
            Some(Utc.ymd(2021, 12, 25).and_hms(00, 15, 23))
        );
        assert_eq!(
            task.get_modified(),
            Some(Utc.ymd(2021, 12, 31).and_hms(17, 56, 14))
        );
        assert!(task.has_tag(&"house".try_into().unwrap()));
        assert!(!task.has_tag(&"PENDING".try_into().unwrap()));
        assert_eq!(
            task.get_annotations().collect::<Vec<_>>(),
            vec![taskchampion::Annotation {
                entry: Utc.ymd(2021, 12, 23).and_hms(14, 20, 31),
                description: "ordered from website".into(),
            }]
        );
        assert_eq!(
            task.get_legacy_uda("githubcreatedon"),
            Some("20211110T175919Z")
        );
        assert_eq!(task.get_legacy_uda("githubnamespace"), Some("djmitche"));
        assert_eq!(task.get_legacy_uda("githubnumber"), Some("228"));

        Ok(())
    }
}

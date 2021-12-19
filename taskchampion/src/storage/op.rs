use crate::server::SyncOp;
use crate::storage::TaskMap;
use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

/// A ReplicaOp defines a single change to the task database, as stored locally in the replica.
/// This contains additional information not included in SyncOp.
#[derive(PartialEq, Clone, Debug, Serialize, Deserialize)]
pub enum ReplicaOp {
    /// Create a new task.
    ///
    /// On undo, the task is deleted.
    Create { uuid: Uuid },

    /// Delete an existing task.
    ///
    /// On undo, the task's data is restored from old_task.
    Delete { uuid: Uuid, old_task: TaskMap },

    /// Update an existing task, setting the given property to the given value.  If the value is
    /// None, then the corresponding property is deleted.
    ///
    /// On undo, the property is set back to its previous value.
    Update {
        uuid: Uuid,
        property: String,
        old_value: Option<String>,
        value: Option<String>,
        timestamp: DateTime<Utc>,
    },
}

impl ReplicaOp {
    /// Convert this operation into a [`SyncOp`].
    pub fn into_sync(self) -> SyncOp {
        match self {
            Self::Create { uuid } => SyncOp::Create { uuid },
            Self::Delete { uuid, .. } => SyncOp::Delete { uuid },
            Self::Update {
                uuid,
                property,
                value,
                timestamp,
                ..
            } => SyncOp::Update {
                uuid,
                property,
                value,
                timestamp,
            },
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use chrono::Utc;
    use pretty_assertions::assert_eq;

    use ReplicaOp::*;

    #[test]
    fn test_json_create() -> anyhow::Result<()> {
        let uuid = Uuid::new_v4();
        let op = Create { uuid };
        let json = serde_json::to_string(&op)?;
        assert_eq!(json, format!(r#"{{"Create":{{"uuid":"{}"}}}}"#, uuid));
        let deser: ReplicaOp = serde_json::from_str(&json)?;
        assert_eq!(deser, op);
        Ok(())
    }

    #[test]
    fn test_json_delete() -> anyhow::Result<()> {
        let uuid = Uuid::new_v4();
        let old_task = vec![("foo".into(), "bar".into())].drain(..).collect();
        let op = Delete { uuid, old_task };
        let json = serde_json::to_string(&op)?;
        assert_eq!(
            json,
            format!(
                r#"{{"Delete":{{"uuid":"{}","old_task":{{"foo":"bar"}}}}}}"#,
                uuid
            )
        );
        let deser: ReplicaOp = serde_json::from_str(&json)?;
        assert_eq!(deser, op);
        Ok(())
    }

    #[test]
    fn test_json_update() -> anyhow::Result<()> {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();

        let op = Update {
            uuid,
            property: "abc".into(),
            old_value: Some("true".into()),
            value: Some("false".into()),
            timestamp,
        };

        let json = serde_json::to_string(&op)?;
        assert_eq!(
            json,
            format!(
                r#"{{"Update":{{"uuid":"{}","property":"abc","old_value":"true","value":"false","timestamp":"{:?}"}}}}"#,
                uuid, timestamp,
            )
        );
        let deser: ReplicaOp = serde_json::from_str(&json)?;
        assert_eq!(deser, op);
        Ok(())
    }

    #[test]
    fn test_json_update_none() -> anyhow::Result<()> {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();

        let op = Update {
            uuid,
            property: "abc".into(),
            old_value: None,
            value: None,
            timestamp,
        };

        let json = serde_json::to_string(&op)?;
        assert_eq!(
            json,
            format!(
                r#"{{"Update":{{"uuid":"{}","property":"abc","old_value":null,"value":null,"timestamp":"{:?}"}}}}"#,
                uuid, timestamp,
            )
        );
        let deser: ReplicaOp = serde_json::from_str(&json)?;
        assert_eq!(deser, op);
        Ok(())
    }

    #[test]
    fn test_into_sync_create() {
        let uuid = Uuid::new_v4();
        assert_eq!(Create { uuid }.into_sync(), SyncOp::Create { uuid });
    }

    #[test]
    fn test_into_sync_delete() {
        let uuid = Uuid::new_v4();
        assert_eq!(
            Delete {
                uuid,
                old_task: TaskMap::new()
            }
            .into_sync(),
            SyncOp::Delete { uuid }
        );
    }

    #[test]
    fn test_into_sync_update() {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();
        assert_eq!(
            Update {
                uuid,
                property: "prop".into(),
                old_value: Some("foo".into()),
                value: Some("v".into()),
                timestamp,
            }
            .into_sync(),
            SyncOp::Update {
                uuid,
                property: "prop".into(),
                value: Some("v".into()),
                timestamp,
            }
        );
    }
}

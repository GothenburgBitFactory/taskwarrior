use crate::server::SyncOp;
use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

/// A ReplicaOp defines a single change to the task database, as stored locally in the replica.
/// This contains additional information not included in SyncOp.
#[derive(PartialEq, Clone, Debug, Serialize, Deserialize)]
pub enum ReplicaOp {
    /// Create a new task.
    ///
    /// On application, if the task already exists, the operation does nothing.
    Create { uuid: Uuid },

    /// Delete an existing task.
    ///
    /// On application, if the task does not exist, the operation does nothing.
    Delete { uuid: Uuid },

    /// Update an existing task, setting the given property to the given value.  If the value is
    /// None, then the corresponding property is deleted.
    ///
    /// If the given task does not exist, the operation does nothing.  
    Update {
        uuid: Uuid,
        property: String,
        value: Option<String>,
        timestamp: DateTime<Utc>,
    },
}

impl ReplicaOp {
    /// Convert this operation into a [`SyncOp`].
    pub fn into_sync(self) -> SyncOp {
        match self {
            Self::Create { uuid } => SyncOp::Create { uuid },
            Self::Delete { uuid } => SyncOp::Delete { uuid },
            Self::Update {
                uuid,
                property,
                value,
                timestamp,
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
        let op = Delete { uuid };
        let json = serde_json::to_string(&op)?;
        assert_eq!(json, format!(r#"{{"Delete":{{"uuid":"{}"}}}}"#, uuid));
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
            value: Some("false".into()),
            timestamp,
        };

        let json = serde_json::to_string(&op)?;
        assert_eq!(
            json,
            format!(
                r#"{{"Update":{{"uuid":"{}","property":"abc","value":"false","timestamp":"{:?}"}}}}"#,
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
            value: None,
            timestamp,
        };

        let json = serde_json::to_string(&op)?;
        assert_eq!(
            json,
            format!(
                r#"{{"Update":{{"uuid":"{}","property":"abc","value":null,"timestamp":"{:?}"}}}}"#,
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
        assert_eq!(Delete { uuid }.into_sync(), SyncOp::Delete { uuid });
    }

    #[test]
    fn test_into_sync_update() {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();
        assert_eq!(
            Update {
                uuid,
                property: "prop".into(),
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

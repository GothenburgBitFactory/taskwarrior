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

    /// Mark a point in the operations history to which the user might like to undo.  Users
    /// typically want to undo more than one operation at a time (for example, most changes update
    /// both the `modified` property and some other task property -- the user would like to "undo"
    /// both updates at the same time).  Applying an UndoPoint does nothing.
    UndoPoint,
}

impl ReplicaOp {
    /// Convert this operation into a [`SyncOp`].
    pub fn into_sync(self) -> Option<SyncOp> {
        match self {
            Self::Create { uuid } => Some(SyncOp::Create { uuid }),
            Self::Delete { uuid, .. } => Some(SyncOp::Delete { uuid }),
            Self::Update {
                uuid,
                property,
                value,
                timestamp,
                ..
            } => Some(SyncOp::Update {
                uuid,
                property,
                value,
                timestamp,
            }),
            Self::UndoPoint => None,
        }
    }

    /// Generate a sequence of SyncOp's to reverse the effects of this ReplicaOp.
    pub fn reverse_ops(self) -> Vec<SyncOp> {
        match self {
            Self::Create { uuid } => vec![SyncOp::Delete { uuid }],
            Self::Delete { uuid, mut old_task } => {
                let mut ops = vec![SyncOp::Create { uuid }];
                // We don't have the original update timestamp, but it doesn't
                // matter because this SyncOp will just be applied and discarded.
                let timestamp = Utc::now();
                for (property, value) in old_task.drain() {
                    ops.push(SyncOp::Update {
                        uuid,
                        property,
                        value: Some(value),
                        timestamp,
                    });
                }
                ops
            }
            Self::Update {
                uuid,
                property,
                old_value,
                timestamp,
                ..
            } => vec![SyncOp::Update {
                uuid,
                property,
                value: old_value,
                timestamp,
            }],
            Self::UndoPoint => vec![],
        }
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use crate::storage::taskmap_with;
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
        assert_eq!(Create { uuid }.into_sync(), Some(SyncOp::Create { uuid }));
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
            Some(SyncOp::Delete { uuid })
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
            Some(SyncOp::Update {
                uuid,
                property: "prop".into(),
                value: Some("v".into()),
                timestamp,
            })
        );
    }

    #[test]
    fn test_into_sync_undo_point() {
        assert_eq!(UndoPoint.into_sync(), None);
    }

    #[test]
    fn test_reverse_create() {
        let uuid = Uuid::new_v4();
        assert_eq!(Create { uuid }.reverse_ops(), vec![SyncOp::Delete { uuid }]);
    }

    #[test]
    fn test_reverse_delete() {
        let uuid = Uuid::new_v4();
        let reversed = Delete {
            uuid,
            old_task: taskmap_with(vec![("prop1".into(), "v1".into())]),
        }
        .reverse_ops();
        assert_eq!(reversed.len(), 2);
        assert_eq!(reversed[0], SyncOp::Create { uuid });
        assert!(matches!(
            &reversed[1],
            SyncOp::Update { uuid: u, property: p, value: Some(v), ..}
                if u == &uuid && p == "prop1" && v == "v1"
        ));
    }

    #[test]
    fn test_reverse_update() {
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
            .reverse_ops(),
            vec![SyncOp::Update {
                uuid,
                property: "prop".into(),
                value: Some("foo".into()),
                timestamp,
            }]
        );
    }

    #[test]
    fn test_reverse_undo_point() {
        assert_eq!(UndoPoint.reverse_ops(), vec![]);
    }
}

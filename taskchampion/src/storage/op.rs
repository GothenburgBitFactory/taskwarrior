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

use ReplicaOp::*;

impl ReplicaOp {
    // Transform takes two operations A and B that happened concurrently and produces two
    // operations A' and B' such that `apply(apply(S, A), B') = apply(apply(S, B), A')`. This
    // function is used to serialize operations in a process similar to a Git "rebase".
    //
    //        *
    //       / \
    //  op1 /   \ op2
    //     /     \
    //    *       *
    //
    // this function "completes the diamond:
    //
    //    *       *
    //     \     /
    // op2' \   / op1'
    //       \ /
    //        *
    //
    // such that applying op2' after op1 has the same effect as applying op1' after op2.  This
    // allows two different systems which have already applied op1 and op2, respectively, and thus
    // reached different states, to return to the same state by applying op2' and op1',
    // respectively.
    pub fn transform(
        operation1: ReplicaOp,
        operation2: ReplicaOp,
    ) -> (Option<ReplicaOp>, Option<ReplicaOp>) {
        match (&operation1, &operation2) {
            // Two creations or deletions of the same uuid reach the same state, so there's no need
            // for any further operations to bring the state together.
            (&Create { uuid: uuid1 }, &Create { uuid: uuid2 }) if uuid1 == uuid2 => (None, None),
            (&Delete { uuid: uuid1 }, &Delete { uuid: uuid2 }) if uuid1 == uuid2 => (None, None),

            // Given a create and a delete of the same task, one of the operations is invalid: the
            // create implies the task does not exist, but the delete implies it exists.  Somewhat
            // arbitrarily, we prefer the Create
            (&Create { uuid: uuid1 }, &Delete { uuid: uuid2 }) if uuid1 == uuid2 => {
                (Some(operation1), None)
            }
            (&Delete { uuid: uuid1 }, &Create { uuid: uuid2 }) if uuid1 == uuid2 => {
                (None, Some(operation2))
            }

            // And again from an Update and a Create, prefer the Update
            (&Update { uuid: uuid1, .. }, &Create { uuid: uuid2 }) if uuid1 == uuid2 => {
                (Some(operation1), None)
            }
            (&Create { uuid: uuid1 }, &Update { uuid: uuid2, .. }) if uuid1 == uuid2 => {
                (None, Some(operation2))
            }

            // Given a delete and an update, prefer the delete
            (&Update { uuid: uuid1, .. }, &Delete { uuid: uuid2 }) if uuid1 == uuid2 => {
                (None, Some(operation2))
            }
            (&Delete { uuid: uuid1 }, &Update { uuid: uuid2, .. }) if uuid1 == uuid2 => {
                (Some(operation1), None)
            }

            // Two updates to the same property of the same task might conflict.
            (
                &Update {
                    uuid: ref uuid1,
                    property: ref property1,
                    value: ref value1,
                    timestamp: ref timestamp1,
                },
                &Update {
                    uuid: ref uuid2,
                    property: ref property2,
                    value: ref value2,
                    timestamp: ref timestamp2,
                },
            ) if uuid1 == uuid2 && property1 == property2 => {
                // if the value is the same, there's no conflict
                if value1 == value2 {
                    (None, None)
                } else if timestamp1 < timestamp2 {
                    // prefer the later modification
                    (None, Some(operation2))
                } else {
                    // prefer the later modification or, if the modifications are the same,
                    // just choose one of them
                    (Some(operation1), None)
                }
            }

            // anything else is not a conflict of any sort, so return the operations unchanged
            (_, _) => (Some(operation1), Some(operation2)),
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
}

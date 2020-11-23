use chrono::{DateTime, Utc};
use serde::{Deserialize, Serialize};
use uuid::Uuid;

/// An Operation defines a single change to the task database
#[derive(PartialEq, Clone, Debug, Serialize, Deserialize)]
pub enum Operation {
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

use Operation::*;

impl Operation {
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
        operation1: Operation,
        operation2: Operation,
    ) -> (Option<Operation>, Option<Operation>) {
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
                } else if timestamp1 > timestamp2 {
                    // prefer the later modification
                    //(Some(operation1), None)
                    (Some(operation1), None)
                } else {
                    // arbitrarily resolve in favor of the first operation
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
    use crate::taskdb::TaskDB;
    use crate::taskstorage::InMemoryStorage;
    use chrono::{Duration, Utc};
    use proptest::prelude::*;

    // note that `tests/operation_transform_invariant.rs` tests the transform function quite
    // thoroughly, so this testing is light.

    fn test_transform(
        setup: Option<Operation>,
        o1: Operation,
        o2: Operation,
        exp1p: Option<Operation>,
        exp2p: Option<Operation>,
    ) {
        let (o1p, o2p) = Operation::transform(o1.clone(), o2.clone());
        assert_eq!((&o1p, &o2p), (&exp1p, &exp2p));

        // check that the two operation sequences have the same effect, enforcing the invariant of
        // the transform function.
        let mut db1 = TaskDB::new_inmemory();
        if let Some(ref o) = setup {
            db1.apply(o.clone()).unwrap();
        }
        db1.apply(o1).unwrap();
        if let Some(o) = o2p {
            db1.apply(o).unwrap();
        }

        let mut db2 = TaskDB::new_inmemory();
        if let Some(ref o) = setup {
            db2.apply(o.clone()).unwrap();
        }
        db2.apply(o2).unwrap();
        if let Some(o) = o1p {
            db2.apply(o).unwrap();
        }

        assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
    }

    #[test]
    fn test_unrelated_create() {
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        test_transform(
            None,
            Create { uuid: uuid1 },
            Create { uuid: uuid2 },
            Some(Create { uuid: uuid1 }),
            Some(Create { uuid: uuid2 }),
        );
    }

    #[test]
    fn test_related_updates_different_props() {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();

        test_transform(
            Some(Create { uuid }),
            Update {
                uuid,
                property: "abc".into(),
                value: Some("true".into()),
                timestamp,
            },
            Update {
                uuid,
                property: "def".into(),
                value: Some("false".into()),
                timestamp,
            },
            Some(Update {
                uuid,
                property: "abc".into(),
                value: Some("true".into()),
                timestamp,
            }),
            Some(Update {
                uuid,
                property: "def".into(),
                value: Some("false".into()),
                timestamp,
            }),
        );
    }

    #[test]
    fn test_related_updates_same_prop() {
        let uuid = Uuid::new_v4();
        let timestamp1 = Utc::now();
        let timestamp2 = timestamp1 + Duration::seconds(10);

        test_transform(
            Some(Create { uuid }),
            Update {
                uuid,
                property: "abc".into(),
                value: Some("true".into()),
                timestamp: timestamp1,
            },
            Update {
                uuid,
                property: "abc".into(),
                value: Some("false".into()),
                timestamp: timestamp2,
            },
            None,
            Some(Update {
                uuid,
                property: "abc".into(),
                value: Some("false".into()),
                timestamp: timestamp2,
            }),
        );
    }

    #[test]
    fn test_related_updates_same_prop_same_time() {
        let uuid = Uuid::new_v4();
        let timestamp = Utc::now();

        test_transform(
            Some(Create { uuid }),
            Update {
                uuid,
                property: "abc".into(),
                value: Some("true".into()),
                timestamp,
            },
            Update {
                uuid,
                property: "abc".into(),
                value: Some("false".into()),
                timestamp,
            },
            Some(Update {
                uuid,
                property: "abc".into(),
                value: Some("true".into()),
                timestamp,
            }),
            None,
        );
    }

    fn uuid_strategy() -> impl Strategy<Value = Uuid> {
        prop_oneof![
            Just(Uuid::parse_str("83a2f9ef-f455-4195-b92e-a54c161eebfc").unwrap()),
            Just(Uuid::parse_str("56e0be07-c61f-494c-a54c-bdcfdd52d2a7").unwrap()),
            Just(Uuid::parse_str("4b7ed904-f7b0-4293-8a10-ad452422c7b3").unwrap()),
            Just(Uuid::parse_str("9bdd0546-07c8-4e1f-a9bc-9d6299f4773b").unwrap()),
        ]
    }

    fn operation_strategy() -> impl Strategy<Value = Operation> {
        prop_oneof![
            uuid_strategy().prop_map(|uuid| Operation::Create { uuid }),
            uuid_strategy().prop_map(|uuid| Operation::Delete { uuid }),
            (uuid_strategy(), "(title|project|status)").prop_map(|(uuid, property)| {
                Operation::Update {
                    uuid,
                    property,
                    value: Some("true".into()),
                    timestamp: Utc::now(),
                }
            }),
        ]
    }

    proptest! {
        #![proptest_config(ProptestConfig {
          cases: 1024, .. ProptestConfig::default()
        })]
        #[test]
        // check that the two operation sequences have the same effect, enforcing the invariant of
        // the transform function.
        fn transform_invariant_holds(o1 in operation_strategy(), o2 in operation_strategy()) {
            let (o1p, o2p) = Operation::transform(o1.clone(), o2.clone());

            let mut db1 = TaskDB::new(Box::new(InMemoryStorage::new()));
            let mut db2 = TaskDB::new(Box::new(InMemoryStorage::new()));

            // Ensure that any expected tasks already exist
            if let Operation::Update{ ref uuid, .. } = o1 {
                let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
                let _ = db2.apply(Operation::Create{uuid: uuid.clone()});
            }

            if let Operation::Update{ ref uuid, .. } = o2 {
                let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
                let _ = db2.apply(Operation::Create{uuid: uuid.clone()});
            }

            if let Operation::Delete{ ref uuid } = o1 {
                let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
                let _ = db2.apply(Operation::Create{uuid: uuid.clone()});
            }

            if let Operation::Delete{ ref uuid } = o2 {
                let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
                let _ = db2.apply(Operation::Create{uuid: uuid.clone()});
            }

            // if applying the initial operations fail, that indicates the operation was invalid
            // in the base state, so consider the case successful.
            if let Err(_) = db1.apply(o1) {
                return Ok(());
            }
            if let Err(_) = db2.apply(o2) {
                return Ok(());
            }

            if let Some(o) = o2p {
                db1.apply(o).map_err(|e| TestCaseError::Fail(format!("Applying to db1: {}", e).into()))?;
            }
            if let Some(o) = o1p {
                db2.apply(o).map_err(|e| TestCaseError::Fail(format!("Applying to db2: {}", e).into()))?;
            }
            assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
        }
    }
}

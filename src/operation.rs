use chrono::{DateTime, Utc};
use serde_json::Value;
use uuid::Uuid;

#[derive(PartialEq, Clone, Debug)]
pub enum Operation {
    Create {
        uuid: Uuid,
    },
    Update {
        uuid: Uuid,
        property: String,
        value: Value,
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
            // Two creations of the same uuid reach the same state, so there's no need for any
            // further operations to bring the state together.
            (&Create { uuid: uuid1 }, &Create { uuid: uuid2 }) if uuid1 == uuid2 => (None, None),

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
    use crate::taskdb::DB;

    fn test_transform(
        o1: Operation,
        o2: Operation,
        exp1p: Option<Operation>,
        exp2p: Option<Operation>,
    ) {
        let (o1p, o2p) = Operation::transform(o1.clone(), o2.clone());
        assert_eq!((&o1p, &o2p), (&exp1p, &exp2p));

        // check that the two operation sequences have the same effect, enforcing the invariant of
        // the transform function.
        let mut db1 = DB::new();
        db1.apply(o1).unwrap();
        if let Some(o) = o2p {
            db1.apply(o).unwrap();
        }
        let mut db2 = DB::new();
        db2.apply(o2).unwrap();
        if let Some(o) = o1p {
            db2.apply(o).unwrap();
        }
        assert_eq!(db1.tasks(), db2.tasks());
    }

    #[test]
    fn test_unrelated_create() {
        let uuid1 = Uuid::new_v4();
        let uuid2 = Uuid::new_v4();

        test_transform(
            Operation::Create { uuid: uuid1 },
            Operation::Create { uuid: uuid2 },
            Some(Operation::Create { uuid: uuid1 }),
            Some(Operation::Create { uuid: uuid2 }),
        );
    }
}

use chrono::Utc;
use proptest::prelude::*;
use taskwarrior_rust::{Operation, DB};
use uuid::Uuid;

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

        let mut db1 = DB::new();

        // Ensure that any expected tasks already exist
        if let Operation::Update{ ref uuid, .. } = o1 {
            let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        if let Operation::Update{ ref uuid, .. } = o2 {
            let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        if let Operation::Delete{ ref uuid } = o1 {
            let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        if let Operation::Delete{ ref uuid } = o2 {
            let _ = db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        let mut db2 = db1.clone();

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
        assert_eq!(db1.tasks(), db2.tasks());
    }
}

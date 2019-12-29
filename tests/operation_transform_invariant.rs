use chrono::Utc;
use ot::Operation;
use ot::DB;
use proptest::prelude::*;
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
    #[test]
    fn transform_invariant_holds(o1 in operation_strategy(), o2 in operation_strategy()) {
        let (o1p, o2p) = Operation::transform(o1.clone(), o2.clone());

        // check that the two operation sequences have the same effect, enforcing the invariant of
        // the transform function.  This needs some care as if either of the operations is
        // an Update then we must ensure the task already exists in the DB.
        let mut db1 = DB::new();

        if let Operation::Update{ ref uuid, .. } = o1 {
            db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        if let Operation::Update{ ref uuid, .. } = o2 {
            db1.apply(Operation::Create{uuid: uuid.clone()});
        }

        let mut db2 = db1.clone();

        db1.apply(o1);
        if let Some(o) = o2p {
            db1.apply(o);
        }
        db2.apply(o2);
        if let Some(o) = o1p {
            db2.apply(o);
        }
        assert_eq!(db1.tasks(), db2.tasks());
    }
}

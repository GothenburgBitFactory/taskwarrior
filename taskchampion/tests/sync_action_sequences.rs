use chrono::Utc;
use proptest::prelude::*;
use taskchampion::{taskstorage, Operation, DB};
use uuid::Uuid;

mod shared;
use shared::TestServer;

fn newdb() -> DB {
    DB::new(Box::new(taskstorage::InMemoryStorage::new()))
}

#[derive(Debug)]
enum Action {
    Op(Operation),
    Sync,
}

fn action_sequence_strategy() -> impl Strategy<Value = Vec<(Action, u8)>> {
    // Create, Update, Delete, or Sync on client 1, 2, .., followed by a round of syncs
    "([CUDS][123])*S1S2S3S1S2".prop_map(|seq| {
        let uuid = Uuid::parse_str("83a2f9ef-f455-4195-b92e-a54c161eebfc").unwrap();
        seq.as_bytes()
            .chunks(2)
            .map(|action_on| {
                let action = match action_on[0] {
                    b'C' => Action::Op(Operation::Create { uuid }),
                    b'U' => Action::Op(Operation::Update {
                        uuid,
                        property: "title".into(),
                        value: Some("foo".into()),
                        timestamp: Utc::now(),
                    }),
                    b'D' => Action::Op(Operation::Delete { uuid }),
                    b'S' => Action::Sync,
                    _ => unreachable!(),
                };
                let acton = action_on[1] - b'1';
                (action, acton)
            })
            .collect::<Vec<(Action, u8)>>()
    })
}

proptest! {
    #[test]
    // check that various sequences of operations on mulitple db's do not get the db's into an
    // incompatible state.  The main concern here is that there might be a sequence of create
    // and delete operations that results in a task existing in one DB but not existing in
    // another.  So, the generated sequences focus on a single task UUID.
    fn transform_sequences_of_operations(action_sequence in action_sequence_strategy()) {
        let mut server = TestServer::new();
        let mut dbs = [newdb(), newdb(), newdb()];

        for (action, db) in action_sequence {
            println!("{:?} on db {}", action, db);

            let db = &mut dbs[db as usize];
            match action {
                Action::Op(op) => {
                    if let Err(e) = db.apply(op) {
                        println!("  {:?} (ignored)", e);
                    }
                },
                Action::Sync => db.sync("me", &mut server).unwrap(),
            }
        }

        assert_eq!(dbs[0].sorted_tasks(), dbs[0].sorted_tasks());
        assert_eq!(dbs[1].sorted_tasks(), dbs[2].sorted_tasks());
    }
}

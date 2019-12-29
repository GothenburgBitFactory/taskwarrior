use chrono::Utc;
use ot::{Operation, Server, DB};
use uuid::Uuid;

#[test]
fn test_sync() {
    let mut server = Server::new();

    let mut db1 = DB::new();
    db1.sync("me", &mut server);

    let mut db2 = DB::new();
    db2.sync("me", &mut server);

    // make some changes in parallel to db1 and db2..
    let uuid1 = Uuid::new_v4();
    db1.apply(Operation::Create { uuid: uuid1 });
    db1.apply(Operation::Update {
        uuid: uuid1,
        property: "title".into(),
        value: "my first task".into(),
        timestamp: Utc::now(),
    });

    let uuid2 = Uuid::new_v4();
    db2.apply(Operation::Create { uuid: uuid2 });
    db2.apply(Operation::Update {
        uuid: uuid2,
        property: "title".into(),
        value: "my second task".into(),
        timestamp: Utc::now(),
    });

    // and synchronize those around
    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.tasks(), db2.tasks());

    // now make updates to the same task on both sides
    db1.apply(Operation::Update {
        uuid: uuid2,
        property: "priority".into(),
        value: "H".into(),
        timestamp: Utc::now(),
    });
    db2.apply(Operation::Update {
        uuid: uuid2,
        property: "project".into(),
        value: "personal".into(),
        timestamp: Utc::now(),
    });

    // and synchronize those around
    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.tasks(), db2.tasks());
}

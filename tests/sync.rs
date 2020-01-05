use chrono::Utc;
use taskwarrior_rust::{taskstorage, Operation, Server, DB};
use uuid::Uuid;

fn newdb() -> DB {
    DB::new(Box::new(taskstorage::InMemoryStorage::new()))
}

#[test]
fn test_sync() {
    let mut server = Server::new();

    let mut db1 = newdb();
    db1.sync("me", &mut server);

    let mut db2 = newdb();
    db2.sync("me", &mut server);

    // make some changes in parallel to db1 and db2..
    let uuid1 = Uuid::new_v4();
    db1.apply(Operation::Create { uuid: uuid1 }).unwrap();
    db1.apply(Operation::Update {
        uuid: uuid1,
        property: "title".into(),
        value: Some("my first task".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    let uuid2 = Uuid::new_v4();
    db2.apply(Operation::Create { uuid: uuid2 }).unwrap();
    db2.apply(Operation::Update {
        uuid: uuid2,
        property: "title".into(),
        value: Some("my second task".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    // and synchronize those around
    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

    // now make updates to the same task on both sides
    db1.apply(Operation::Update {
        uuid: uuid2,
        property: "priority".into(),
        value: Some("H".into()),
        timestamp: Utc::now(),
    })
    .unwrap();
    db2.apply(Operation::Update {
        uuid: uuid2,
        property: "project".into(),
        value: Some("personal".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    // and synchronize those around
    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
}

#[test]
fn test_sync_create_delete() {
    let mut server = Server::new();

    let mut db1 = newdb();
    db1.sync("me", &mut server);

    let mut db2 = newdb();
    db2.sync("me", &mut server);

    // create and update a task..
    let uuid = Uuid::new_v4();
    db1.apply(Operation::Create { uuid }).unwrap();
    db1.apply(Operation::Update {
        uuid: uuid,
        property: "title".into(),
        value: Some("my first task".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    // and synchronize those around
    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());

    // delete and re-create the task on db1
    db1.apply(Operation::Delete { uuid }).unwrap();
    db1.apply(Operation::Create { uuid }).unwrap();
    db1.apply(Operation::Update {
        uuid: uuid,
        property: "title".into(),
        value: Some("my second task".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    // and on db2, update a property of the task
    db2.apply(Operation::Update {
        uuid: uuid,
        property: "project".into(),
        value: Some("personal".into()),
        timestamp: Utc::now(),
    })
    .unwrap();

    db1.sync("me", &mut server);
    db2.sync("me", &mut server);
    db1.sync("me", &mut server);
    assert_eq!(db1.sorted_tasks(), db2.sorted_tasks());
}

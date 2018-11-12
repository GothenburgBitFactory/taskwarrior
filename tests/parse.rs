extern crate taskwarrior;
extern crate chrono;

use std::fs::File;
use std::io::BufReader;
use chrono::prelude::*;

#[test]
fn test_parse() {
    let filename = "tests/data/tdb2-test.data";
    let file = File::open(filename).unwrap();
    let tasks = taskwarrior::parse(filename, BufReader::new(file)).unwrap();
    assert_eq!(
        tasks[0].description,
        "https://phabricator.services.example.com/D7364 [taskgraph] Download debian packages"
    );
    assert_eq!(tasks[0].entry, Utc.timestamp(1538520624, 0));
    assert_eq!(tasks[0].udas.get("phabricatorid").unwrap(), "D7364");
    assert_eq!(tasks[1].annotations[0].entry, Utc.timestamp(1541461824, 0));
    assert!(tasks[1].annotations[0].description.starts_with(
        "https://github.com",
    ));
}

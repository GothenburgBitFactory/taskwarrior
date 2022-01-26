use lazy_static::lazy_static;
use std::sync::Mutex;

lazy_static! {
    // the C library running the tests is not reentrant, so we use a mutex to ensure that only one
    // test runs at a time.
    static ref MUTEX: Mutex<()> = Mutex::new(());
}

macro_rules! suite(
    { $s:ident } => {
    #[test]
    fn $s() {
        let _guard = MUTEX.lock().unwrap();
        assert_eq!(integration_tests::bindings_tests::$s(), 0);
    }
    };
);

include!(concat!(env!("OUT_DIR"), "/bindings_test_suites.rs"));

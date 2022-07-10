use lazy_static::lazy_static;
use std::sync::Mutex;
use tempfile::TempDir;

lazy_static! {
    // the C library running the tests is not reentrant, so we use a mutex to ensure that only one
    // test runs at a time.
    static ref MUTEX: Mutex<()> = Mutex::new(());
}

macro_rules! suite(
    { $s:ident } => {
    #[test]
    fn $s() {
        let tmp_dir = TempDir::new().expect("TempDir failed");
        let (res, output) = {
            let _guard = MUTEX.lock().unwrap();
            // run the tests in the temp dir (NOTE: this must be inside
            // the mutex guard!)
            std::env::set_current_dir(tmp_dir.as_ref()).unwrap();
            integration_tests::bindings_tests::$s()
        };
        println!("{}", output);
        if res != 0 {
            assert!(false, "test failed");
        }
    }
    };
);

include!(concat!(env!("OUT_DIR"), "/bindings_test_suites.rs"));

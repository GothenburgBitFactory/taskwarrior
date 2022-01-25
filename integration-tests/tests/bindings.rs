macro_rules! suite(
    { $s:ident } => {
    #[test]
    fn $s() {
        assert_eq!(integration_tests::bindings_tests::$s(), 0);
    }
    };
);

// keep this list in sync with integration-tests/build.rs and
// integration-tests/src/bindings_tests/mod.rs
suite!(uuid_tests);
suite!(string_tests);
suite!(task_tests);
suite!(replica_tests);

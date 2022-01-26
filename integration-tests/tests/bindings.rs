macro_rules! suite(
    { $s:ident } => {
    #[test]
    fn $s() {
        assert_eq!(integration_tests::bindings_tests::$s(), 0);
    }
    };
);

include!(concat!(env!("OUT_DIR"), "/bindings_test_suites.rs"));

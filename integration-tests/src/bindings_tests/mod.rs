use std::fs;

extern "C" {
    // set up to send test output to TEST-OUTPUT
    fn setup_output();
    // close the output file
    fn finish_output();
}

// Each suite is represented by a <name>_tests C function in <name>.c.
// All of these C files are built into a library that is linked to the crate -- but not to test
// crates.  So, this macro produces a "glue function" that calls the C function, and that can be
// called from test crates.
macro_rules! suite(
    { $s:ident } => {
    pub fn $s() -> (i32, String) {
        extern "C" {
            fn $s() -> i32;
        }
        unsafe { setup_output() };
        let res = unsafe { $s() };
        unsafe { finish_output() };
        let output = fs::read_to_string("TEST-OUTPUT")
            .unwrap_or_else(|e| format!("could not open TEST-OUTPUT: {}", e));
        (res, output)
    }
    };
);

include!(concat!(env!("OUT_DIR"), "/bindings_test_suites.rs"));

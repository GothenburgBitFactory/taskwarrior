// Each suite is represented by a <name>_tests C function in <name>.c.
// All of these C files are built into a library that is linked to the crate -- but not to test
// crates.  So, this macro produces a "glue function" that calls the C function, and that can be
// called from test crates.
macro_rules! suite(
    { $s:ident } => {
    pub fn $s() -> i32 {
        extern "C" {
            fn $s() -> i32;
        }
        unsafe { $s() }
    }
    };
);

// keep this list in sync with integration-tests/build.rs and integration-tests/tests/bindings.rs.
suite!(uuid_tests);
suite!(string_tests);
suite!(task_tests);
suite!(replica_tests);

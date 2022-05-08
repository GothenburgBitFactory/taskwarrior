# Integration Tests for Taskchampion

## "Regular" Tests

Some of the tests in `tests/` are just regular integration tests.
Nothing exciting to see.

## Bindings Tests

The bindings tests are a bit more interesting, since they are written in C.
They are composed of a collection of "suites", each in one C file in `integration-tests/src/bindings_tests/`.
Each suite contains a number of tests (using [Unity](http://www.throwtheswitch.org/unity)) and an exported function named after the suite that returns an exit status (1 = failure).

The build script (`integration-tests/build.rs`) builds these files into a library that is linked with the `integration_tests` library crate.
This crate contains a `bindings_tests` module with a pub function for each suite.

Finally, the `integration-tests/tests/bindings.rs` test file calls each of those functions in a separate test case.

### Adding Tests

To add a test, select a suite and add a new test-case function.
Add a `RUN_TEST` invocation for your new function to the `.._tests` function at the bottom.
Keep the `RUN_TEST`s in the same order as the functions they call.

### Adding Suites

To add a suite,

1. Add a new C file in `integration-tests/src/bindings_tests/`, based off of one of the others.
1. Add a the suite name to `suites` in `integration-tests/build.rs`.

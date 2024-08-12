## Running Tests
Do this to run all tests:
```shell
cmake --build build --target test_runner --target task_executable
ctest --test-dir build
```

All tests produce TAP (Test Anything Protocol) output.
In order to run the tests in parallel add the `--parallel <# of threads>` or shortly `-j <# of threads>` option to `ctest`.
Depending on your IDE, all tests might also be available under the `All CTest` target.
Keep in mind that the tests are not automatically rebuild if a source file is changes, it requires a manual rebuild.

Please also have a look at [development.md](../doc/devel/contrib/development.md) for more information on how to run tests as well as some information about `ctest`.

## Architecture

There are three varieties of tests:

  * C++ unit tests that test low-level object interfaces. These are typically
    very fast tests, and are exhaustive in nature.

  * Python unit tests that are at the highest level, exercising the command
    line, hooks and syncing. There is an example, 'template.test.py', that
    shows how to perform various high level tests.

  * Bash unit tests, one test per file, using the bash_tap_tw.sh script. These
    tests are small, quick tests, not intended to be permanent.

All tests are named with the pattern '*.test.py', '*.test.sh', or '*.test.cpp',
and any other forms are not run by the test harness.
In the case of Python tests one can still run them manually by launching them with 'python testname.test.py' or simply './testname.test.py'.

If a test is failing and can not be fixed, it can be marked as `WILL_FAIL` in the `CMakeLists.txt` file.
See the [WILL_FAIL](https://cmake.org/cmake/help/latest/prop_test/WILL_FAIL.html) documentation for more information.
However, please keep in mind that such tests should be fixed as soon as possible as well as proper documentation should be added to the issue tracker.

It also allows us to keep tests submitted for bugs that are not scheduled to be fixed in the upcoming release, and we don't want
the failing tests to prevent us from seeing 100% pass rate for the bugs we *have* fixed.


## Goals

The test suite is evolving, and becoming a better tool for determining whether
code is ready for release. There are goals that shape these changes, and they
are:

  * Increase test coverage by testing more features, more thoroughly. The test
    coverage level is (as of 2016-07-24) at 86.5%.

  * Write fewer bug regression tests. Over time, bug regression tests are less
    useful than feature tests, and more likely to contain overlapping coverage.

  * Eliminate obsolete tests, which are tests that have overlapping coverage.
    There is simply no point in testing a feature twice, in the same manner.


## What Makes a Good Test

A good test ensures that a feature is functioning as expected, and contains
both positive and negative aspects, or in other words looks for expected
behavior as well as looking for the absence of unexpected behavior.


## Conventions for writing a test

If you wish to contribute tests, please consider the following guidelines:

  * For a new bug, an accompanying test is very helpful.  Suppose you write up
    a bug, named TW-1234, then the test would be a script named tw-1234.t, and
    based on the template.t example.

    Over time, we will migrate the tests in tw-1234.t into a feature-specific
    test script, such as filter.t, export.t, whichever is appropriate.

  * Tests created after bugs or feature requests should (ideally) have an entry
    on https://github.com/GothenburgBitFactory/taskwarrior/issues and should
    include the issue ID in a docstring or comment.

  * Class and method names should be descriptive of what they are testing.
    Example: TestFilterOnReports

  * Docstrings on Python tests are mandatory. The first line is used as title
    of the test. Include the issue ID - there are many examples of this.

  * Extra information and details should go into multi-line docstrings or
    comments.

  * Python tests for bugs or features not yet fixed/implemented should be
    decorated with: @unittest.skip("WaitingFor TW-xxxx"). We would rather have
    a live test that is skipped, than no test.


## How to Submit a Test Change/Addition

Mail it to support@gothenburgbitfactory.org, or attach it to an open bug.


## Wisdom

Here are some guildelines that may help:

  * If there are any lexer.t tests failing, then ignore all the others and fix
    these first. They are fundamental and affect everything else. One Lexer
    failure can cause 30 symptomatic failures, and addressing any of those is
    wrong.

  * If any of the C++ tests fail, fix them next, for the same reason as above.

  * If you are about to fix a bug, and no tests are failing, add tests that fail
    in a script named tw-XXXX.t. Later, someone will incorporate that test
    script into higher-level feature tests.

  * If the command line parser is not working, start by blaming the Lexer.

  * While the lowest level (C++) tests should be exhaustive, higher level tests
    should not do the same by iterating over the entire problem space. It is a
    waste of time.

  * If you find that you are combining two features into one test, you are
    probably doing it wrong.

  * If you add a feature, then add a test to prove it works, also add a test to
    prove it doesn't simultaneously generate errors. Furthermore test that with
    the feature disabled, or command line arguments missing, appropriate errors
    are reported.


## TODO

For anyone looking for test-related tasks to take on, here are some suggestions:

  * Find and eliminate duplicate tests.

  * Using <attribute>.startswith:<value> with rc.regex:off still uses regex.

  * Crazy dateformat values are not tested.

  * Invalid UTF8 is not tested.

  * All the attribute modifiers need to be tested, only a few are.

  * Aliases are not well tested, and fragile.

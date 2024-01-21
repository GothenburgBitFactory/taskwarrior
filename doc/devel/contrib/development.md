# Developing Taskwarrior

The following describes the process for developing Taskwarrior. If you are only
changing TaskChampion (Rust code), you can simply treat it like any other Rust
project: modify the source under `taskchampion/` and use `cargo test` to run
the TaskChampion tests.

See the [TaskChampion CONTRIBUTING guide](../../../taskchampion/CONTRIBUTING.md) for more.

## Satisfy the Requirements:

 * CMake 3.0 or later
 * gcc 7.0 or later, clang 6.0 or later, or a compiler with full C++17 support
 * libuuid (if not on macOS)
 * python 3 (optional, for running the test suite)
 * Rust 1.64.0 or higher (hint: use https://rustup.rs/ instead of using your system's package manager)

## Obtain and Build Code:
```
    $ git clone --recursive https://github.com/GothenburgBitFactory/taskwarrior taskwarrior.git
    $ cd taskwarrior.git
    $ git checkout develop               # Latest dev branch
    $ git submodule init                 # This is now done by cmake as a test
    $ git submodule update               # Update the libhsared.git submodule
    $ cmake -DCMAKE_BUILD_TYPE=debug .   # debug or release. Default: neither
    $ make VERBOSE=1 -j4                 # Shows details, builds using 4 jobs
                                         # Alternately 'export MAKEFLAGS=-j 4'
```

This will build several executables, but the one you want is probably `src/task`.
When you make changes, just run the last line again.

## Run the Test Suite:

First switch to the test directory:

```
    $ cd test
```
Then you can run all tests, showing details, with
```
    $ make VERBOSE=1
```
Alternately, run the tests with the details hidden in `all.log`:
```
    $ ./run_all
```
Either way, you can get a summary of any test failures with:
```
    $ ./problems
```

Note that any development should be performed using a git clone, and the current development branch.
The source tarballs do not reflect HEAD, and do not contain the test suite.
Follow the [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow) for creating a pull request.

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
 * Rust 1.64.0 or higher (hint: use https://rustup.rs/ instead of using your system's package manager)

## Install Optional Dependencies:
 * python 3 (for running the test suite)
 * clangd or ccls (for C++ integration in many editors)
 * rust-analyzer (for Rust integration in many editors)

## Obtain and Build Code:
The following documentation works with CMake 3.14 and later.
Here are the minimal steps to get started, using an out of source build directory and calling the underlying build tool over the CMake interface.
See the general CMake man pages or the [cmake-documentation](https://cmake.org/cmake/help/latest/manual/cmake.1.html) for more,

## Basic Building
```sh
git clone https://github.com/GothenburgBitFactory/taskwarrior
cd taskwarrior
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```
Other possible build types can be `Release` and `Debug`.
This will build several executables, but the one you want is probably `src/task`, located in the `build` directory.
When you make changes, just run the last line again.

### Building a specific target
For **only** building the `task` executable, use
```sh
cmake --build build --target task_executable
```

### Building in parallel
If a parallel build is wanted use
```sh
cmake --build build -j <number-of-jobs>
```

### Building with clang as compiler
```sh
cmake -S . -B build-clang\
    -DCMAKE_C_COMPILER=clang\
    -DCMAKE_CXX_COMPILER=clang++
cmake --build build-clang
```

## Run the Test Suite:
First switch to the test directory:

```
    $ cd build/test
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

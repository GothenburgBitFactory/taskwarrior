# Developing Taskwarrior

## Satisfy the Requirements:

 * CMake 3.22 or later
 * gcc 7.0 or later, clang 6.0 or later, or a compiler with full C++17 support
 * libuuid (if not on macOS)
 * Rust 1.64.0 or higher (hint: use https://rustup.rs/ instead of using your system's package manager)

## Install Optional Dependencies:
 * python 3 (for running the test suite)
 * pre-commit (for applying formatting changes locally)
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
For running the test suite [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) is used.
Before one can run the test suite the `task_executable` must be built.
After that also the `test_runner` target must be build, which can be done over:
```sh
cmake --build build --target test_runner
```
Again you may also use the `-j <number-of-jobs>` option for parallel builds.

Now one can invoke `ctest` to run the tests.
```sh
ctest --test-dir build
```
This would run all the test in serial and might take some time.

### Running tests in parallel
```sh
ctest --test-dir build -j <number-of-jobs>
```

Further it is adviced to add the `--output-on-failure` option to `ctest`, to recieve a verbose output if a test is failing as well as the `--rerun-failed` flag, to invoke in subsequent runs only the failed ones.

### Running specific tests
For this case one can use the `-R <regex>` or `--tests-regex <regex>`  option to run only the tests matching the regular expression.
Running only the `cpp` tests can then be achieved over
```sh
ctest --test-dir build -R cpp
```
or running the `variant_*` tests
```sh
ctest --test-dir build -R variant
```

### Repeating a test case
In order to find sporadic test failures the `--repeat` flag can be used.
```sh
ctest --test-dir build -R cpp --repeat-until-fail 10
```

There are more options to `ctest` such as `--progress`, allowing to have a less verbose output.
They can be found in the [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) man page.

Note that any development should be performed using a git clone, and the current development branch.
The source tarballs do not reflect HEAD, and do not contain the test suite.
Follow the [GitHub flow](https://docs.github.com/en/get-started/quickstart/github-flow) for creating a pull request.

## Using a Custom Version of TaskChampion

To build against a different version of Taskchampion, modify the requirement in `src/taskchampion-cpp/Cargo.toml`.

To build from a local checkout, replace the version with a [path dependency](https://doc.rust-lang.org/cargo/reference/specifying-dependencies.html#specifying-path-dependencies), giving the path to the directory containing TaskChampion's `Cargo.toml`:

```toml
taskchampion = { path = "path/to/taskchampion" }
```

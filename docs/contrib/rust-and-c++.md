# Rust and C++

Taskwarrior has historically been a C++ project, but as part of an [ongoing effort to replace its storage backend](https://github.com/GothenburgBitFactory/taskwarrior/issues/2770) it now includes a Rust library called TaskChampion.
To develop Taskwarrior, you will need both a [C++ compiler and tools](./development.md), and a [Rust toolchain](https://www.rust-lang.org/tools/install).
However, most tasks will only require you to be familiar with one of the two languages.

## TaskChampion

TaskChampion implements storage and access to "replicas" containing a user's tasks.
It defines an abstract model for this data, and also provides a simple Rust API for manipulating replicas.
It also defines a method of synchronizing replicas and provides an implementation of that method in the form of a sync server.
TaskChampion provides a C interface via the `taskchampion-lib` crate.

Other applications, besides Taskwarrior, can use TaskChampion to manage tasks.
Applications written in Rust can use the `taskchampion` crate, while those in other languages may use the `taskchampion-lib` crate.
Taskwarrior is just one application using the TaskChampion interface.

You can build Taskchampion locally by simply running `cargo build` in the root of this repository.
The implementation, including more documentation, is in the [`rust`](../../rust) subdirectory.

## Taskwarrior's use of TaskChampion

Taskwarrior's interface to TaskChampion has a few laters:

* The skeletal Rust crate in [`src/tc/rust`](../../src/tc/rust) brings the symbols from `taskchampion-lib` under CMake's management.
  The corresponding header file is included from [`rust/lib`](../../rust/lib).
  All of these symbols are placed in the C++ namespace, `tc::ffi`.
* C++ wrappers for the types from `taskchampion-lib` are defined in [`src/tc`](../../src/tc), ensuring memory safety (with `unique_ptr`) and adding methods corresponding to the Rust API's methods.
  The wrapper types are in the C++ namespace, `tc`.

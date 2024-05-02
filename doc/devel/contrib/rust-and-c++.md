# Rust and C++

Taskwarrior has historically been a C++ project, but as of taskwarrior-3.0.0, the storage backend is now provided by a Rust library called TaskChampion.

## TaskChampion

TaskChampion implements storage and access to "replicas" containing a user's tasks.
It defines an abstract model for this data, and also provides a simple Rust API for manipulating replicas.
It also defines a method of synchronizing replicas and provides an implementation of that method in the form of a sync server.
TaskChampion provides a C interface via the `taskchampion-lib` crate, at `src/tc/lib`.

Other applications, besides Taskwarrior, can use TaskChampion to manage tasks.
Taskwarrior is just one application using the TaskChampion interface.

## Taskwarrior's use of TaskChampion

Taskwarrior's interface to TaskChampion has a few layers:

* A Rust library, `takschampion-lib`, that presents `extern "C"` functions for use from C++, essentially defining a C interface to TaskChampion.
* C++ wrappers for the types from `taskchampion-lib`, defined in [`src/tc`](../../src/tc), ensuring memory safety (with `unique_ptr`) and adding methods corresponding to the Rust API's methods.
  The wrapper types are in the C++ namespace, `tc`.

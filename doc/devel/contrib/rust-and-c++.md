# Rust and C++

Taskwarrior has historically been a C++ project, but as of taskwarrior-3.0.0, the storage backend is now provided by a Rust library called TaskChampion.

## TaskChampion

TaskChampion implements storage and access to "replicas" containing a user's tasks.
It defines an abstract model for this data, and also provides a simple Rust API for manipulating replicas.
It also defines a method of synchronizing replicas and provides an implementation of that method in the form of a sync server.

Other applications, besides Taskwarrior, can use TaskChampion to manage tasks.
Taskwarrior is just one application using the TaskChampion interface.

## Taskwarrior's use of TaskChampion

Taskwarrior's interface to TaskChampion is in `src/taskchampion-cpp`.
This links to `taskchampion` as a Rust dependency, and uses [cxx](https://cxx.rs) to build a C++ API for it.
That API is defined, and documented, in `src/taskchampion-cpp/src/lib.rs`, and available in the `tc` namespace in C++ code.

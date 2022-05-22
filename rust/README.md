TaskChampion
------------

TaskChampion is an open-source personal task-tracking application.
Use it to keep track of what you need to do, with a quick command-line interface and flexible sorting and filtering.
It is modeled on [TaskWarrior](https://taskwarrior.org), but not a drop-in replacement for that application.

See the [documentation](https://taskchampion.github.io/taskchampion/) for more!

## Status

TaskChampion currently functions as a "testbed" for new functionality that may later be incorporated into TaskWarrior.
It can be developed without the requirements of compatibliity, allowing us to explore and fix edge-cases in things like the replica-synchronization model.

While you are welcome to [help out](https://github.com/taskchampion/taskchampion/blob/main/CONTRIBUTING.md), you should do so with the awareness that your work might never be used.
But, if you just want to get some practice with Rust, we'd be happy to have you.

## Structure

There are five crates here:

 * [taskchampion](./taskchampion) - the core of the tool
 * [taskchampion-cli](./cli) - the command-line binary
 * [taskchampion-sync-server](./sync-server) - the server against which `task sync` operates
 * [taskchampion-lib](./lib) - glue code to use _taskchampion_ from C
 * [integration-tests](./integration-tests) - integration tests covering _taskchampion-cli_, _taskchampion-sync-server_, and _taskchampion-lib_.

## Code Generation

The _taskchampion_lib_ crate uses a bit of code generation to create the `lib/taskchampion.h` header file.
To regenerate this file, run `cargo xtask codegen`.

## C libraries

NOTE: support for linking against taskchampion is a work in progress.
Contributions and pointers to best practices are appreciated!

The `taskchampion-lib` crate generates libraries suitable for use from C (or any C-compatible language).

The necessary bits are:

* a shared object in `target/$PROFILE/deps` (e.g., `target/debug/deps/libtaskchampion.so`)
* a static library in `target/$PROFILE` (e.g., `target/debug/libtaskchampion.a`)
* a header file, `lib/taskchampion.h`.

Downstream consumers may use either the static or dynamic library, as they prefer.

NOTE: on Windows, the "BCrypt" library must be included when linking to taskchampion.

### As a Rust dependency

If you would prefer to build Taskchampion directly into your project, and have a build system capable of building Rust libraries (such as CMake), the `taskchampion-lib` crate can be referenced as an `rlib` dependency.

TaskChampion
------------

TaskChampion implements the task storage and synchronization behind Taskwarrior.
It includes an implementation with Rust and C APIs, allowing any application to maintain and manipulate its own replica.
It also includes a specification for tasks and how thye are synchronized, inviting alternative implementations of replicas or task servers.

See the [documentation](https://taskchampion.github.io/taskchampion/) for more!

NOTE: Taskwarrior is currently in the midst of a change to use TaskChampion as its storage.
Until that is complete, the information here may be out-of-date.

## Structure

There are five crates here:

 * [taskchampion](./taskchampion) - the core of the tool
 * [taskchampion-sync-server](./sync-server) - the server against which `task sync` operates
 * [taskchampion-lib](./lib) - glue code to use _taskchampion_ from C
 * [integration-tests](./integration-tests) (private) - integration tests covering _taskchampion-cli_, _taskchampion-sync-server_, and _taskchampion-lib_.
 * [xtask](./xtask) (private) - implementation of the `cargo xtask codegen` command

## Code Generation

The _taskchampion_lib_ crate uses a bit of code generation to create the `lib/taskchampion.h` header file.
To regenerate this file, run `cargo xtask codegen`.

## Rust API

The Rust API, as defined in [the docs](https://docs.rs/taskchampion/latest/taskchampion/), supports simple creation and manipulation of of replicas and the tasks they contain.

The Rust API follows semantic versioning.
As this is still in the `0.x` phase, so breaking changes may occur but will be indicated with a change to the minor version.

## C API

The `taskchampion-lib` crate generates libraries suitable for use from C (or any C-compatible language).
It is a "normal" Cargo crate that happens to export a number of `extern "C"` symbols, and also contains a `taskchampion.h` defining those symbols.

*WARNING: the C API is not yet stable!*

It is your responsibility to link this into a form usable in your own build process.
For example, in a typical CMake C++ project, CMakeRust can do this for you.
In many cases, this is as simple as a rust crate with `src/lib.rs` containing

```rust
pub use taskchampion_lib::*;
```

Arrange to use the header file, `lib/taskchampion.h`, by copying it or adding its directory to your include search path.
[Future work](https://github.com/GothenburgBitFactory/taskwarrior/issues/2870) will provide better automation for tihs process.

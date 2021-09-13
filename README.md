TaskChampion
------------

TaskChampion is an open-source personal task-tracking application.
Use it to keep track of what you need to do, with a quick command-line interface and flexible sorting and filtering.
It is modeled on [TaskWarrior](https://taskwarrior.org), but not a drop-in replacement for that application.

See the [documentation](https://taskchampion.github.io/taskchampion/) for more!

## Status

TC is still under development.
You are welcome to [help out!](https://github.com/djmitche/taskchampion/blob/main/CONTRIBUTING.md).
Even if you just want to get some practice with Rust, your contribution is welcome.

Since development of TaskChampion began, TaskWarrior developers have resumed work and made several releases.
Assuming that continues, it is unlikely that TaskChampion will ever be recommended for day-to-day use, as that would only serve to split the TaskWarrior community.

## Goals

 * Feature parity with TaskWarrior (but not compatibility)
 * Aproachable, maintainable codebase
 * Active development community
 * Reasonable privacy: user's task details not visible on server
 * Reliable concurrency - clients do not diverge
 * Storage performance O(n) with n number of tasks

## Structure

There are four crates here:

 * [taskchampion](./taskchampion) - the core of the tool
 * [taskchampion-cli](./cli) - the command-line binary
 * [taskchampion-sync-server](./sync-server) - the server against which `task sync` operates
 * [replica-server-tests](./replica-server-tests) - integration tests covering both _taskchampion-cli_ and _taskchampion-sync-server_

## Documentation Generation

The `mdbook` configuration contains a "preprocessor" implemented in the `taskchampion-cli` crate in order to reflect CLI usage information into the generated book.
Tihs preprocessor is not built by default.
To (re)build it, run `cargo build -p taskchampion-cli --features usage-docs --bin usage-docs`.

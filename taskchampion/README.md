TaskChampion
------------

TaskChampion is an open-source personal task-tracking application.
Use it to keep track of what you need to do, with a quick command-line interface and flexible sorting and filtering.
It is modeled on [TaskWarrior](https://taskwarrior.org), but not a drop-in replacement for that application.

Goals:

 * Feature parity with TaskWarrior (but not compatibility)
 * Aproachable, maintainable codebase
 * Active development community
 * Reasonable privacy: user's task details not visible on server
 * Reliable concurrency - clients do not diverge
 * Storage performance O(n) with n number of tasks

See:

 * [Documentation](docs/src/SUMMARY.md) (will be published as an mdbook eventually)
 * [Progress on the first version](https://github.com/djmitche/taskwarrior-rust/projects/1)

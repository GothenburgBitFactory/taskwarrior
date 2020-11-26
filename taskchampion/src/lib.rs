/*!

This crate implements the core of TaskChampion, the [replica](crate::Replica).

A TaskChampion replica is a local copy of a user's task data.  As the name suggests, several
replicas of the same data can exist (such as on a user's laptop and on their phone) and can
synchronize with one another.

# Task Storage

The [`taskstorage`](crate::taskstorage) module supports pluggable storage for a replica's data.
An implementation is provided, but users of this crate can provide their own implementation as well.

# Server

Replica synchronization takes place against a server.
The [`server`](crate::server) module defines the interface a server must meet.

# See Also

See the [TaskChampion Book](https://github.com/djmitche/taskchampion/blob/main/docs/src/SUMMARY.md)
for more information about the design and usage of the tool.

 */

mod errors;
mod replica;
pub mod server;
mod task;
mod taskdb;
pub mod taskstorage;
mod utils;

pub use replica::Replica;
pub use task::Priority;
pub use task::Status;
pub use task::{Task, TaskMut};

/// Re-exported type from the `uuid` crate, for ease of compatibility for consumers of this crate.
pub use uuid::Uuid;

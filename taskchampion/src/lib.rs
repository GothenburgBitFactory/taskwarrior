#![deny(clippy::all)]
/*!

This crate implements the core of TaskChampion, the [replica](crate::Replica).

# Replica

A TaskChampion replica is a local copy of a user's task data.  As the name suggests, several
replicas of the same data can exist (such as on a user's laptop and on their phone) and can
synchronize with one another.

Replicas are accessed using the [`Replica`](crate::replica) type.

# Task Storage

The [`storage`](crate::storage) module supports pluggable storage for a replica's data.
An implementation is provided, but users of this crate can provide their own implementation as well.

# Server

Replica synchronization takes place against a server.
Create a server with [`ServerConfig`](crate::ServerConfig).

The [`server`](crate::server) module defines the interface a server must meet.
Users can define their own server impelementations.

# See Also

See the [TaskChampion Book](http://djmitche.github.com/taskchampion)
for more information about the design and usage of the tool.

 */

mod config;
mod errors;
mod replica;
pub mod server;
pub mod storage;
mod task;
mod taskdb;
mod utils;
mod workingset;

pub use config::ReplicaConfig;
pub use replica::Replica;
pub use server::{Server, ServerConfig};
pub use task::{Priority, Status, Tag, Task, TaskMut};
pub use workingset::WorkingSet;

/// Re-exported type from the `uuid` crate, for ease of compatibility for consumers of this crate.
pub use uuid::Uuid;

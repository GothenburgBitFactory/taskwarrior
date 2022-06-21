#![deny(clippy::all)]
/*!

This crate implements the core of TaskChampion, the [replica](crate::Replica).

Users of this crate can manipulate a task database using this API, including synchronizing that task database with others via a synchronization server.

Example uses of this crate:
 * user interfaces for task management, such as mobile apps, web apps, or command-line interfaces
 * integrations for task management, such as synchronization with ticket-tracking systems or
   request forms.

# Replica

A TaskChampion replica is a local copy of a user's task data.  As the name suggests, several
replicas of the same data can exist (such as on a user's laptop and on their phone) and can
synchronize with one another.

Replicas are accessed using the [`Replica`](crate::Replica) type.

# Task Storage

Replicas access the task database via a [storage object](crate::storage::Storage).
Create a storage object with [`StorageConfig`](crate::storage::StorageConfig).

The [`storage`](crate::storage) module supports pluggable storage for a replica's data.
An implementation is provided, but users of this crate can provide their own implementation as well.

# Server

Replica synchronization takes place against a server.
Create a server with [`ServerConfig`](crate::ServerConfig).

The [`server`](crate::server) module defines the interface a server must meet.
Users can define their own server impelementations.

# See Also

See the [TaskChampion Book](http://taskchampion.github.com/taskchampion)
for more information about the design and usage of the tool.

# Minimum Supported Rust Version (MSRV)

This crate supports Rust version 1.61 and higher.

 */

// NOTE: it's important that this 'mod' comes first so that the macros can be used in other modules
mod macros;

mod depmap;
mod errors;
mod replica;
pub mod server;
pub mod storage;
mod task;
mod taskdb;
mod utils;
mod workingset;

pub use depmap::DependencyMap;
pub use errors::Error;
pub use replica::Replica;
pub use server::{Server, ServerConfig};
pub use storage::StorageConfig;
pub use task::{Annotation, Status, Tag, Task, TaskMut};
pub use workingset::WorkingSet;

/// Re-exported type from the `uuid` crate, for ease of compatibility for consumers of this crate.
pub use uuid::Uuid;

/// Re-exported chrono module.
pub use chrono;

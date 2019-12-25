Sketch for a Taskd Replacement
------------------------------

Goals:

 * Reasonable privacy: user's task details not visible on server
 * Reliable concurrency - clients do not diverge
 * Storage O(n) with n number of tasks

# Data Model

A client manages a single offline instance of a single user's task list.
The data model is only seen from the clients' perspective.

## Task Database

The task database is composed of an un-ordered collection of tasks, each keyed by a UUID.
Each task has an arbitrary-sized set of key/value properties, with JSON values.
A property with a `null` value is considered equivalent to that property not being set on the task.

Tasks are only created, never deleted.
See below for details on how tasks can "expire" from the task database.

## Operations

Every change to the task database is captured as an operation.
Each operation has one of the forms 
 * `Create(uuid)`
 * `Update(uuid, property, value, timestamp)`

The former form creates a new task.
The latter form updates the given property of the given task.
It is invalid to update a task that does not exist.
The timestamp on updates serves as additional metadata and is used to resolve conflicts.

Operations act as deltas between database states.

## Versions and Synchronization

Occasionally, database states are named with an integer, called a version.
The system as a whole (server and clients) constructs a monotonic sequence of versions and the operations that separate each version from the next.
No gaps are allowed in the verison numbering.
Version 0 is implicitly the empty database.

The server stores the operations for each version, and provides them as needed to clients.
Clients use this information to update their local task databases, and to generate new versions to send to the server.

Clients generate a new version to transmit changes made locally to the server.
The changes are represented as a sequence of operations with the final operation being tagged as the version.
In order to keep the gap-free monotonic numbering, the server will only accept a proposed version from a client if its number is one greater that the latest version on the server.
When this is not the case, the client must "rebase" the local changes onto the latest version from the server and try again.
This operation is performed using operational transformation (OT).
The result of this transformation is a sequence of operations based on the latest version, and a sequence of operations the client can apply to its local task database to "catch up" to the version on the server.

## Snapshots

As designed, storage required on the server would grow with time, as would the time required for new clients to update to the latest version.
As an optimization, the server also stores "snapshots" containing a full copy of the task database at a given version.
Based on configurable heuristics, it may delete older operations and snapshots, as long as enough data remains for active clients to synchronize and for new clients to initialize.

Since snapshots must be computed by clients, the server may "request" a snapshot when providing the latest version to a client.
This request comes with a number indicating how much it 'wants" the snapshot.
Clients which can easily generate and transmit a snapshot should be generous to the server, while clients with more limited resources can wait until the server's requests are more desperate.
The intent is, where possible, to request snapshots created on well-connected desktop clients over mobile and low-power clients.

## Encryption and Signing

From the server's perspective, all data except for version numbers are opaque binary blobs.
Clients encrypt and sign these blobs using a symmetric key known only to the clients.
This secures the data at-rest on the server.
Note that privacy is not complete, as the server still has some information about users, including source and frequency of synchronization transactions and size of those transactions.

## Expiration

TBD

.. conditions on flushing to allow consistent handling

# Implementation Notes

## Client / Server Protocol

TBD

.. using HTTP
.. user auth
.. user setup process

## Batching Operations

TBD

## Recurrence

TBD


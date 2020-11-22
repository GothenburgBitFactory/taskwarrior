# Synchronization

The [task database](./taskdb.md) also implements synchronization.
Synchronization occurs between disconnected replicas, mediated by a server.
The replicas never communicate directly with one another.
The server does not have access to the task data; it sees only opaque blobs of data with a small amount of metadata.

The synchronization process is a critical part of the task database's functionality, and it cannot function efficiently without occasional synchronization operations

## Operational Transformations

Synchronization is based on [operational transformation](https://en.wikipedia.org/wiki/Operational_transformation).
This section will assume some familiarity with the concept.

## State and Operations

At a given time, the set of tasks in a replica's storage is the essential "state" of that replica.
All modifications to that state occur via operations, as defined in [Replica Storage](./storage.md).
We can draw a network, or graph, with the nodes representing states and the edges representing operations.
For example:

```text
  o -- State: {abc-d123: 'get groceries', priority L}
  |
  | -- Operation: set abc-d123 priority to H
  |
  o -- State: {abc-d123: 'get groceries', priority H}
```

For those familiar with distributed version control systems, a state is analogous to a revision, while an operation is analogous to a commit.

Fundamentally, synchronization involves all replicas agreeing on a single, linear sequence of operations and the state that those operations create.
Since the replicas are not connected, each may have additional operations that have been applied locally, but which have not yet been agreed on.
The synchronization process uses operational transformation to "linearize" those operations.
This process is analogous (vaguely) to rebasing a sequence of Git commits.

### Versions

Occasionally, database states are named with an integer, called a version.
The system as a whole (all replicas) constructs a monotonic sequence of versions and the operations that separate each version from the next.
No gaps are allowed in the version numbering.
Version 0 is implicitly the empty database.

The server stores the operations to change a state from a version N to a version N+1, and provides that information as needed to replicas.
Replicas use this information to update their local task databases, and to generate new versions to send to the server.

Replicas generate a new version to transmit changes made locally to the server.
The changes are represented as a sequence of operations with the state resulting from the final operation corresponding to the version.
In order to keep the gap-free monotonic numbering, the server will only accept a proposed version from a replica if its number is one greater that the latest version on the server.

In the non-conflict case (such as with a single replica), then, a replica's synchronization process involves gathering up the operations it has accumulated since its last synchronization; bundling those operations into version N+1; and sending that version to the server.

### Transformation

When the latest version on the server contains operations that are not present in the replica, then the states have diverged.
For example (with lower-case letters designating operations):

```text
  o  -- version N
 w|\a
  o o
 x|  \b
  o   o
 y|    \c
  o     o -- replica's local state
 z|
  o -- version N+1
```

In this situation, the replica must "rebase" the local operations onto the latest version from the server and try again.
This process is performed using operational transformation (OT).
The result of this transformation is a sequence of operations based on the latest version, and a sequence of operations the replica can apply to its local task database to reach the same state
Continuing the example above, the resulting operations are shown with `'`:

```text
  o  -- version N
 w|\a
  o o
 x|  \b
  o   o
 y|    \c
  o     o -- replica's intermediate local state
 z|     |w'
  o-N+1 o
 a'\    |x'
    o   o
   b'\  |y'
      o o
     c'\|z'
        o  -- version N+2
```

The replica applies w' through z' locally, and sends a' through c' to the server as the operations to generate version N+2.
Either path through this graph, a-b-c-w'-x'-y'-z' or a'-b'-c'-w-x-y-z, must generate *precisely* the same final state at version N+2.
Careful selection of the operations and the transformation function ensure this.

See the comments in the source code for the details of how this transformation process is implemented.

## Replica Implementation

The replica's [storage](./storage.md) contains the current state in `tasks`, the as-yet un-synchronized operations in `operations`, and the last version at which synchronization occurred in `base_version`.

To perform a synchronization, the replica first requests any versions greater than `base_version` from the server, and rebases any local operations on top of those new versions, updating `base_version`.
If there are no un-synchronized local operations, the process is complete.
Otherwise, the replica creates a new version containing those local operations and uploads that to the server.
In most cases, this will succeed, but if another replica has created a new version in the interim, then the new version will conflict with that other replica's new version.
In this case, the process repeats.

The replica's un-synchronized operations are already reflected in `tasks`, so the following invariant holds:

> Applying `operations` to the set of tasks at `base_version` gives a set of tasks identical
> to `tasks`.

## Server Implementation

The server implementation is simple.
It supports fetching versions keyed by number, and adding a new version.
In adding a new version, the version number must be one greater than the greatest existing version.

Critically, the server operates on nothing more than numbered, opaque blobs of data.

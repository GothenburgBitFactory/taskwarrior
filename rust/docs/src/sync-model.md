# Synchronization Model

The [task database](./taskdb.md) also implements synchronization.
Synchronization occurs between disconnected replicas, mediated by a server.
The replicas never communicate directly with one another.
The server does not have access to the task data; it sees only opaque blobs of data with a small amount of metadata.

The synchronization process is a critical part of the task database's functionality, and it cannot function efficiently without occasional synchronization operations

## Operational Transforms

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

### Sync Operations

The [Replica Storage](./storage.md) model contains additional information in its operations that is not included in operations synchronized to other replicas.
In this document, we will be discussing "sync operations" of the form

 * `Create(uuid)`
 * `Delete(uuid)`
 * `Update(uuid, property, value, timestamp)`


### Versions

Occasionally, database states are given a name (that takes the form of a UUID).
The system as a whole (all replicas) constructs a branch-free sequence of versions and the operations that separate each version from the next.
The version with the nil UUID is implicitly the empty database.

The server stores the operations to change a state from a "parent" version to a "child" version, and provides that information as needed to replicas.
Replicas use this information to update their local task databases, and to generate new versions to send to the server.

Replicas generate a new version to transmit local changes to the server.
The changes are represented as a sequence of operations with the state resulting from the final operation corresponding to the version.
In order to keep the versions in a single sequence, the server will only accept a proposed version from a replica if its parent version matches the latest version on the server.

In the non-conflict case (such as with a single replica), then, a replica's synchronization process involves gathering up the operations it has accumulated since its last synchronization; bundling those operations into a version; and sending that version to the server.

### Replica Invariant

The replica's [storage](./storage.md) contains the current state in `tasks`, the as-yet un-synchronized operations in `operations`, and the last version at which synchronization occurred in `base_version`.

The replica's un-synchronized operations are already reflected in its local `tasks`, so the following invariant holds:

> Applying `operations` to the set of tasks at `base_version` gives a set of tasks identical
> to `tasks`.

### Transformation

When the latest version on the server contains operations that are not present in the replica, then the states have diverged.
For example:

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

(diagram notation: `o` designates a state, lower-case letters designate operations, and versions are presented as if they were numbered sequentially)

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

## Synchronization Process

To perform a synchronization, the replica first requests the child version of `base_version` from the server (GetChildVersion).
It applies that version to its local `tasks`, rebases its local `operations` as described above, and updates `base_version`.
The replica repeats this process until the server indicates no additional child versions exist.
If there are no un-synchronized local operations, the process is complete.

Otherwise, the replica creates a new version containing its local operations, giving its `base_version` as the parent version, and transmits that to the server (AddVersion).
In most cases, this will succeed, but if another replica has created a new version in the interim, then the new version will conflict with that other replica's new version and the server will respond with the new expected parent version.
In this case, the process repeats.
If the server indicates a conflict twice with the same expected base version, that is an indication that the replica has diverged (something serious has gone wrong).

## Servers

A replica depends on periodic synchronization for performant operation.
Without synchronization, its list of pending operations would grow indefinitely, and tasks could never be expired.
So all replicas, even "singleton" replicas which do not replicate task data with any other replica, must synchronize periodically.

TaskChampion provides a `LocalServer` for this purpose.
It implements the `get_child_version` and `add_version` operations as described, storing data on-disk locally, all within the `ta` binary.

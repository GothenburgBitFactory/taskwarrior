# Snapshots

The basic synchronization model described in the previous page has a few shortcomings:
 * servers must store an ever-increasing quantity of versions
 * a new replica must download all versions since the beginning in order to derive the current state

Snapshots allow TaskChampion to avoid both of these issues.
A snapshot is a copy of the task database at a specific version.
It is created by a replica, encrypted, and stored on the server.
A new replica can simply download a recent snapshot and apply any additional versions synchronized since that snapshot was made.
Servers can delete and reclaim space used by older versions, as long as newer snapshots are available.

## Snapshot Heuristics

A server implementation must answer a few questions:
 * How often should snapshots be made?
 * When can versions be deleted?
 * When can snapshots be deleted?

A critical invariant is that at least one snapshot must exist for any database that does not have a child of the nil version.
This ensures that a new replica can always derive the latest state.

Aside from that invariant, the server implementation can vary in its answers to these questions, with the following considerations:

Snapshots should be made frequently enough that a new replica can initialize quickly.

Existing replicas will fail to synchronize if they request a child version that has been deleted.
This failure can cause data loss if the replica had local changes.
It's conceivable that replicas may not sync for weeks or months if, for example, they are located on a home computer while the user is on holiday.

## Requesting New Snapshots

The server requests snapshots from replicas, indicating an urgency for the request.
Some replicas, such as those running on PCs or servers, can produce a snapshot even at low urgency.
Other replicas, in more restricted environments such as mobile devices, will only produce a snapshot at high urgency.
This saves resources in these restricted environments.

A snapshot must be made on a replica with no unsynchronized operations.
As such, it only makes sense to request a snapshot in response to a successful AddVersion request.

## Handling Deleted Versions

When a replica requests a child version, the response must distinguish two cases:

 1. No such child version exists because the replica is up-to-date.
 1. No such child version exists because it has been deleted, and the replica must re-initialize itself.

The details of this logic are covered in the [Server-Replica Protocol](./sync-protocol.md).

# Planned Functionality

This section is a bit of a to-do list for additional functionality to add to the synchronzation system.
Each feature has some discussion of how it might be implemented.

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

## Backups

In this design, the server is little more than an authenticated storage for encrypted blobs provided by the client.
To allow for failure or data loss on the server, clients are expected to cache these blobs locally for a short time (a week), along with a server-provided HMAC signature.
When data loss is detected -- such as when a client expects the server to have a version N or higher, and the server only has N-1, the client can send those blobs to the server.
The server can validate the HMAC and, if successful, add the blobs to its datastore.

## Expiration

Deleted tasks remain in the task database, and are simply hidden in most views.
All tasks have an expiration time after which they may be flushed, preventing unbounded increase in task database size.
However, purging of a task does not satisfy the necessary OT guarantees, so some further formal design work is required before this is implemented.

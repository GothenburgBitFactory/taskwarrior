# Server-Replica Protocol

The server-replica protocol is defined abstractly in terms of request/response transactions from the replica to the server.
This is made concrete in an HTTP representation.

The protocol builds on the model presented in the previous chapter, and in particular on the synchronization process.

## Clients

From the server's perspective, replicas are indistinguishable, so this protocol uses the term "client" to refer generically to all replicas replicating a single task history.

Each client is identified and authenticated with a "client key", known only to the server and to the replicas replicating the task history.

## Server

For each client, the server is responsible for storing the task history, in the form of a branch-free sequence of versions.

For each client, it stores a set of versions as well as the latest version ID, defaulting to the nil UUID.
Each version has a version ID, a parent version ID, and a history segment (opaque data containing the operations for that version).
The server should maintain the following invariants:

1. Given a client c, c.latestVersion is nil or exists in the set of versions.
1. Given versions v1 and v2 for a client, with v1.versionId != v2.versionId and v1.parentVersionId != nil, v1.parentVersionId != v2.parentVersionId.
   In other words, versions do not branch.

Note that versions form a linked list beginning with the version stored in he client. 
This linked list need not continue back to a version with v.parentVersionId = nil.
It may end at any point when v.parentVersionId is not found in the set of Versions.
This observation allows the server to discard older versions.

## Transactions

### AddVersion

The AddVersion transaction requests that the server add a new version to the client's task history.
The request contains the following;

 * parent version ID
 * history segment

The server determines whether the new version is acceptable, atomically with respect to other requests for the same client.
If it has no versions for the client, it accepts the version.
If it already has one or more versions for the client, then it accepts the version only if the given parent version ID matches its stored latest parent ID.

If the version is accepted, the server generates a new version ID for it.
The version is added to the set of versions for the client, the client's latest version ID is set to the new version ID.
The new version ID is returned in the response to the client.

If the version is not accepted, the server makes no changes, but responds to the client with a conflict indication containing the latest version ID.
The client may then "rebase" its operations and try again.
Note that if a client receives two conflict responses with the same parent version ID, it is an indication that the client's version history has diverged from that on the server.

### GetChildVersion

The GetChildVersion transaction is a read-only request for a version.
The request consists of a parent version ID.
The server searches its set of versions for a version with the given parent ID.
If found, it returns the version's

 * version ID,
 * parent version ID (matching that in the request), and
 * history segment.

If not found, the server returns a negative response.

## HTTP Representation

The transactions above are realized for an HTTP server at `<origin>` using the HTTP requests and responses described here.
The `origin` *should* be an HTTPS endpoint on general principle, but nothing in the functonality or security of the protocol depends on connection encryption.

The replica identifies itself to the server using a `clientKey` in the form of a UUID.
This value is passed with every request in the `X-Client-Id` header, in its dashed-hex format.

### AddVersion

The request is a `POST` to `<origin>/client/add-version/<parentVersionId>`.
The request body contains the history segment, optionally encoded using any encoding supported by actix-web.
The content-type must be `application/vnd.taskchampion.history-segment`.

The success response is a 200 OK with an empty body.
The new version ID appears in the `X-Version-Id` header.

On conflict, the response is a 409 CONFLICT with an empty body.
The expected parent version ID appears in the `X-Parent-Version-Id` header.

Other error responses (4xx or 5xx) may be returned and should be treated appropriately to their meanings in the HTTP specification.

### GetChildVersion

The request is a `GET` to `<origin>/client/get-child-version/<parentVersionId>`.
The response is 404 NOT FOUND if no such version exists.
Otherwise, the response is a 200 OK.
The version's history segment is returned in the response body, with content-type `application/vnd.taskchampion.history-segment`.
The version ID appears in the `X-Version-Id` header.
The response body may be encoded, in accordance with any `Accept-Encoding` header in the request.

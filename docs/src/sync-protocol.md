# Server-Replica Protocol

The server-replica protocol is defined abstractly in terms of request/response transactions from the replica to the server.
This is made concrete in an HTTP representation.

The protocol builds on the model presented in the previous chapter, and in particular on the synchronization process.

## Clients

From the server's perspective, replicas accessing the same task history are indistinguishable, so this protocol uses the term "client" to refer generically to all replicas replicating a single task history.

Each client is identified and authenticated with a "client key", known only to the server and to the replicas replicating the task history.

## Server

For each client, the server is responsible for storing the task history, in the form of a branch-free sequence of versions.
It also stores the latest snapshot, if any exists.

 * versions: a set of {versionId: UUID, parentVersionId: UUID, historySegment: bytes}
 * latestVersionId: UUID
 * snapshotVersionId: UUID
 * snapshot: bytes

For each client, it stores a set of versions as well as the latest version ID, defaulting to the nil UUID.
Each version has a version ID, a parent version ID, and a history segment (opaque data containing the operations for that version).
The server should maintain the following invariants for each client:

1. latestVersionId is nil or exists in the set of versions.
2. Given versions v1 and v2 for a client, with v1.versionId != v2.versionId and v1.parentVersionId != nil, v1.parentVersionId != v2.parentVersionId.
   In other words, versions do not branch.
3. If snapshotVersionId is nil, then there is a version with parentVersionId == nil.
4. If snapshotVersionId is not nil, then there is a version with parentVersionId = snapshotVersionId.

Note that versions form a linked list beginning with the latestVersionId stored for the client. 
This linked list need not continue back to a version with v.parentVersionId = nil.
It may end at any point when v.parentVersionId is not found in the set of Versions.
This observation allows the server to discard older versions.
The third invariant prevents the server from discarding versions if there is no snapshot.
The fourth invariant prevents the server from discarding versions newer than the snapshot.

## Data Formats

### Encryption

The client configuration includes an encryption secret of arbitrary length and a clientId to identify itself.
This section describes how that information is used to encrypt and decrypt data sent to the server (versions and snapshots).

#### Key Derivation

The client derives the 32-byte encryption key from the configured encryption secret using PBKDF2 with HMAC-SHA256 and 100,000 iterations.
The salt is the SHA256 hash of the 16-byte form of the client key.

#### Encryption

The client uses [AEAD](https://commondatastorage.googleapis.com/chromium-boringssl-docs/aead.h.html), with algorithm CHACHA20_POLY1305.
The client should generate a random nonce, noting that AEAD is _not secure_ if a nonce is used repeatedly for the same key.

AEAD supports additional authenticated data (AAD) which must be provided for both open and seal operations.
In this protocol, the AAD is always 17 bytes of the form:
 * `app_id` (byte) - always 1
 * `version_id` (16 bytes) - 16-byte form of the version ID associated with this data
   * for versions (AddVersion, GetChildVersion), the _parent_ version_id
   * for snapshots (AddSnapshot, GetSnapshot), the snapshot version_id

The `app_id` field is for future expansion to handle other, non-task data using this protocol.
Including it in the AAD ensures that such data cannot be confused with task data.

Although the AEAD specification distinguishes ciphertext and tags, for purposes of this specification they are considered concatenated into a single bytestring as in BoringSSL's `EVP_AEAD_CTX_seal`.

#### Representation

The final byte-stream is comprised of the following structure:

* `version` (byte) - format version (always 1)
* `nonce` (12 bytes) - encryption nonce
* `ciphertext` (remaining bytes) - ciphertext from sealing operation

The `version` field identifies this data format, and future formats will have a value other than 1 in this position.

### Version

The decrypted form of a version is a JSON array containing operations in the order they should be applied.
Each operation has the form `{TYPE: DATA}`, for example:

 * `{"Create":{"uuid":"56e0be07-c61f-494c-a54c-bdcfdd52d2a7"}}`
 * `{"Delete":{"uuid":"56e0be07-c61f-494c-a54c-bdcfdd52d2a7"}}`
 * `{"Update":{"uuid":"56e0be07-c61f-494c-a54c-bdcfdd52d2a7","property":"prop","value":"v","timestamp":"2021-10-11T12:47:07.188090948Z"}}`
 * `{"Update":{"uuid":"56e0be07-c61f-494c-a54c-bdcfdd52d2a7","property":"prop","value":null,"timestamp":"2021-10-11T12:47:07.188090948Z"}}` (to delete a property)

Timestamps are in RFC3339 format with a `Z` suffix.

### Snapshot

The decrypted form of a snapshot is a JSON object mapping task IDs to task properties.
For example (pretty-printed for clarity):

```json
{
 "56e0be07-c61f-494c-a54c-bdcfdd52d2a7": {
   "description": "a task",
   "priority": "H"
 },
 "4b7ed904-f7b0-4293-8a10-ad452422c7b3": {
   "description": "another task"
 }
}
```

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
The response may also include a request for a snapshot, with associated urgency.

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

The response is either a version (success, _not-found_, or _gone_, as determined by the first of the following to apply:
* If a version with parentVersionId equal to the requested parentVersionId exists, it is returned.
* If the requested parentVersionId is the nil UUID ..
  * ..and snapshotVersionId is nil, the response is _not-found_ (the client has no versions).
  * ..and snapshotVersionId is not nil, the response is _gone_ (the first version has been deleted).
* If a version with versionId equal to the requested parentVersionId exists, the response is _not-found_ (the client is up-to-date)
* Otherwise, the response is _gone_ (the requested version has been deleted).

### AddSnapshot

The AddSnapshot transaction requests that the server store a new snapshot, generated by the client.
The request contains the following:

 * version ID at which the snapshot was made
 * snapshot data (opaque to the server)

The server should validate that the snapshot is for an existing version and is newer than any existing snapshot.
It may also validate that the snapshot is for a "recent" version (e.g., one of the last 5 versions).
If a snapshot already exists for the given version, the server may keep or discard the new snapshot but should return a success indication to the client.

The server response is empty.

### GetSnapshot

The GetSnapshot transaction requests that the server provide the latest snapshot.
The response contains the snapshot version ID and the snapshot data, if those exist.

## HTTP Representation

The transactions above are realized for an HTTP server at `<origin>` using the HTTP requests and responses described here.
The `origin` *should* be an HTTPS endpoint on general principle, but nothing in the functonality or security of the protocol depends on connection encryption.

The replica identifies itself to the server using a `clientKey` in the form of a UUID.
This value is passed with every request in the `X-Client-Id` header, in its dashed-hex format.

### AddVersion

The request is a `POST` to `<origin>/v1/client/add-version/<parentVersionId>`.
The request body contains the history segment, optionally encoded using any encoding supported by actix-web.
The content-type must be `application/vnd.taskchampion.history-segment`.

The success response is a 200 OK with an empty body.
The new version ID appears in the `X-Version-Id` header.
If included, a snapshot request appears in the `X-Snapshot-Request` header with value `urgency=low` or `urgency=high`.

On conflict, the response is a 409 CONFLICT with an empty body.
The expected parent version ID appears in the `X-Parent-Version-Id` header.

Other error responses (4xx or 5xx) may be returned and should be treated appropriately to their meanings in the HTTP specification.

### GetChildVersion

The request is a `GET` to `<origin>/v1/client/get-child-version/<parentVersionId>`.

The response is determined as described above.
The _not-found_ response is 404 NOT FOUND.
The _gone_ response is 410 GONE.
Neither has a response body.

On success, the response is a 200 OK.
The version's history segment is returned in the response body, with content-type `application/vnd.taskchampion.history-segment`.
The version ID appears in the `X-Version-Id` header.
The response body may be encoded, in accordance with any `Accept-Encoding` header in the request.

On failure, a client should treat a 404 NOT FOUND as indicating that it is up-to-date.
Clients should treat a 410 GONE as a synchronization error.
If the client has pending changes to send to the server, based on a now-removed version, then those changes cannot be reconciled and will be lost.
The client should, optionally after consulting the user, download and apply the latest snapshot.

### AddSnapshot

The request is a `POST` to `<origin>/v1/client/add-snapshot/<versionId>`.
The request body contains the snapshot data, optionally encoded using any encoding supported by actix-web.
The content-type must be `application/vnd.taskchampion.snapshot`.

If the version is invalid, as described above, the response should be 400 BAD REQUEST.
The server response should be 200 OK on success.

### GetSnapshot

The request is a `GET` to `<origin>/v1/client/snapshot`.

The response is a 200 OK.
The snapshot is returned in the response body, with content-type `application/vnd.taskchampion.snapshot`.
The version ID appears in the `X-Version-Id` header.
The response body may be encoded, in accordance with any `Accept-Encoding` header in the request.

After downloading and decrypting a snapshot, a client must replace its entire local task database with the content of the snapshot.
Any local operations that had not yet been synchronized must be discarded.
After the snapshot is applied, the client should begin the synchronization process again, starting from the snapshot version.

# HTTP Representation

The transactions in the sync protocol are realized for an HTTP server at `<origin>` using the HTTP requests and responses described here.
The `origin` *should* be an HTTPS endpoint on general principle, but nothing in the functonality or security of the protocol depends on connection encryption.

The replica identifies itself to the server using a `client_id` in the form of a UUID.
This value is passed with every request in the `X-Client-Id` header, in its dashed-hex format.

The salt used in key derivation is the SHA256 hash of the 16-byte form of the client ID.

## AddVersion

The request is a `POST` to `<origin>/v1/client/add-version/<parentVersionId>`.
The request body contains the history segment, optionally encoded using any encoding supported by actix-web.
The content-type must be `application/vnd.taskchampion.history-segment`.

The success response is a 200 OK with an empty body.
The new version ID appears in the `X-Version-Id` header.
If included, a snapshot request appears in the `X-Snapshot-Request` header with value `urgency=low` or `urgency=high`.

On conflict, the response is a 409 CONFLICT with an empty body.
The expected parent version ID appears in the `X-Parent-Version-Id` header.

Other error responses (4xx or 5xx) may be returned and should be treated appropriately to their meanings in the HTTP specification.

## GetChildVersion

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

## AddSnapshot

The request is a `POST` to `<origin>/v1/client/add-snapshot/<versionId>`.
The request body contains the snapshot data, optionally encoded using any encoding supported by actix-web.
The content-type must be `application/vnd.taskchampion.snapshot`.

If the version is invalid, as described above, the response should be 400 BAD REQUEST.
The server response should be 200 OK on success.

## GetSnapshot

The request is a `GET` to `<origin>/v1/client/snapshot`.

The response is a 200 OK.
The snapshot is returned in the response body, with content-type `application/vnd.taskchampion.snapshot`.
The version ID appears in the `X-Version-Id` header.
The response body may be encoded, in accordance with any `Accept-Encoding` header in the request.

After downloading and decrypting a snapshot, a client must replace its entire local task database with the content of the snapshot.
Any local operations that had not yet been synchronized must be discarded.
After the snapshot is applied, the client should begin the synchronization process again, starting from the snapshot version.

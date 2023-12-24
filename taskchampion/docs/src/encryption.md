# Encryption

The client configuration includes an encryption secret of arbitrary length.
This section describes how that information is used to encrypt and decrypt data sent to the server (versions and snapshots).

Encryption is not used for local (on-disk) sync, but is used for all cases where data is sent from the local host.

## Key Derivation

The client derives the 32-byte encryption key from the configured encryption secret using PBKDF2 with HMAC-SHA256 and 100,000 iterations.
The salt value depends on the implemenation of the protocol, as described in subsequent chapters.

## Encryption

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

## Representation

The final byte-stream is comprised of the following structure:

* `version` (byte) - format version (always 1)
* `nonce` (12 bytes) - encryption nonce
* `ciphertext` (remaining bytes) - ciphertext from sealing operation

The `version` field identifies this data format, and future formats will have a value other than 1 in this position.

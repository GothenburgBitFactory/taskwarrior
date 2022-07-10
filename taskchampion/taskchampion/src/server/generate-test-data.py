# This file generates test-encrypted.data.  To run it:
# - pip install cryptography pbkdf2
# - python taskchampion/src/server/generate-test-data.py taskchampion/src/server/

import os
import hashlib
import pbkdf2
import secrets
import sys
import uuid

from cryptography.hazmat.primitives.ciphers.aead import ChaCha20Poly1305

# these values match values used in the rust tests
client_key = "0666d464-418a-4a08-ad53-6f15c78270cd"
encryption_secret = b"b4a4e6b7b811eda1dc1a2693ded"
version_id = "b0517957-f912-4d49-8330-f612e73030c4"

def gen(
    version_id=version_id, client_key=client_key, encryption_secret=encryption_secret,
    app_id=1, version=1):
    # first, generate the encryption key
    salt = hashlib.sha256(uuid.UUID(client_key).bytes).digest()
    key = pbkdf2.PBKDF2(
        encryption_secret,
        salt,
        digestmodule=hashlib.sha256,
        iterations=100000,
    ).read(32)

    # create a nonce
    nonce = secrets.token_bytes(12)

    assert len(b"\x01") == 1
    # create the AAD
    aad = b''.join([
        bytes([app_id]),
        uuid.UUID(version_id).bytes,
    ])

    # encrypt using AEAD
    chacha = ChaCha20Poly1305(key)
    ciphertext = chacha.encrypt(nonce, b"SUCCESS", aad)

    # create the envelope
    envelope = b''.join([
        bytes([version]),
        nonce,
        ciphertext,
    ])

    return envelope


def main():
    dir = sys.argv[1]

    with open(os.path.join(dir, 'test-good.data'), "wb") as f:
        f.write(gen())

    with open(os.path.join(dir, 'test-bad-version-id.data'), "wb") as f:
        f.write(gen(version_id=uuid.uuid4().hex))

    with open(os.path.join(dir, 'test-bad-client-key.data'), "wb") as f:
        f.write(gen(client_key=uuid.uuid4().hex))

    with open(os.path.join(dir, 'test-bad-secret.data'), "wb") as f:
        f.write(gen(encryption_secret=b"xxxxxxxxxxxxxxxxxxxxx"))

    with open(os.path.join(dir, 'test-bad-version.data'), "wb") as f:
        f.write(gen(version=99))

    with open(os.path.join(dir, 'test-bad-app-id.data'), "wb") as f:
        f.write(gen(app_id=99))


main()

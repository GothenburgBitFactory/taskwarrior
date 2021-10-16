/// This module implements the encryption specified in the sync-protocol
/// document.
use byteorder::{NetworkEndian, ReadBytesExt, WriteBytesExt};
use ring::{aead, digest, pbkdf2, rand, rand::SecureRandom};
use std::io::{Cursor, Read};
use uuid::Uuid;

const PBKDF2_ITERATIONS: u32 = 100000;
const ENVELOPE_VERSION: u32 = 1;

/// An Cryptor stores a secret and allows sealing and unsealing.  It derives a key from the secret,
/// which takes a nontrivial amount of time, so it should be created once and re-used for the given
/// client_key.
pub(super) struct Cryptor {
    key: aead::LessSafeKey,
    rng: rand::SystemRandom,
}

impl Cryptor {
    pub(super) fn new(client_key: Uuid, secret: &Secret) -> anyhow::Result<Self> {
        Ok(Cryptor {
            key: Self::derive_key(client_key, secret)?,
            rng: rand::SystemRandom::new(),
        })
    }

    /// Derive a key as specified for version 1.  Note that this may take 10s of ms.
    fn derive_key(client_key: Uuid, secret: &Secret) -> anyhow::Result<aead::LessSafeKey> {
        let salt = digest::digest(&digest::SHA256, client_key.as_bytes());

        let mut key_bytes = vec![0u8; ring::aead::AES_256_GCM.key_len()];
        pbkdf2::derive(
            pbkdf2::PBKDF2_HMAC_SHA256,
            std::num::NonZeroU32::new(PBKDF2_ITERATIONS).unwrap(),
            salt.as_ref(),
            secret.as_ref(),
            &mut key_bytes,
        );

        let unbound_key = ring::aead::UnboundKey::new(&ring::aead::AES_256_GCM, &key_bytes)
            .map_err(|_| anyhow::anyhow!("error while creating AEAD key"))?;
        Ok(ring::aead::LessSafeKey::new(unbound_key))
    }

    /// Encrypt the given payload.
    pub(super) fn seal(&self, payload: Unsealed) -> anyhow::Result<Sealed> {
        let Unsealed {
            version_id,
            mut payload,
        } = payload;

        let mut nonce_buf = [0u8; aead::NONCE_LEN];
        self.rng
            .fill(&mut nonce_buf)
            .map_err(|_| anyhow::anyhow!("error generating random nonce"))?;
        let nonce = ring::aead::Nonce::assume_unique_for_key(nonce_buf);

        let aad = ring::aead::Aad::from(version_id.as_bytes());

        let tag = self
            .key
            .seal_in_place_separate_tag(nonce, aad, &mut payload)
            .map_err(|_| anyhow::anyhow!("error while sealing"))?;
        payload.extend_from_slice(tag.as_ref());

        let env = Envelope {
            nonce: &nonce_buf,
            payload: payload.as_ref(),
        };

        Ok(Sealed {
            version_id,
            payload: env.to_bytes(),
        })
    }

    /// Decrypt the given payload, verifying it was created for the given version_id
    pub(super) fn unseal(&self, payload: Sealed) -> anyhow::Result<Unsealed> {
        let Sealed {
            version_id,
            payload,
        } = payload;

        let env = Envelope::from_bytes(&payload)?;

        let mut nonce = [0u8; aead::NONCE_LEN];
        nonce.copy_from_slice(env.nonce);
        let nonce = ring::aead::Nonce::assume_unique_for_key(nonce);
        let aad = ring::aead::Aad::from(version_id.as_bytes());

        let mut payload = env.payload.to_vec();
        let plaintext = self
            .key
            .open_in_place(nonce, aad, payload.as_mut())
            .map_err(|_| anyhow::anyhow!("error while creating AEAD key"))?;

        Ok(Unsealed {
            version_id,
            payload: plaintext.to_vec(),
        })
    }
}

/// Secret represents a secret key as used for encryption and decryption.
pub(super) struct Secret(pub(super) Vec<u8>);

impl From<Vec<u8>> for Secret {
    fn from(bytes: Vec<u8>) -> Self {
        Self(bytes)
    }
}

impl AsRef<[u8]> for Secret {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

/// Envelope for the data stored on the server, containing the information
/// required to decrypt.
#[derive(Debug, PartialEq, Eq)]
struct Envelope<'a> {
    nonce: &'a [u8],
    payload: &'a [u8],
}

impl<'a> Envelope<'a> {
    fn from_bytes(buf: &'a [u8]) -> anyhow::Result<Envelope<'a>> {
        if buf.len() <= 4 + aead::NONCE_LEN {
            anyhow::bail!("envelope is too small");
        }

        let mut rdr = Cursor::new(buf);
        let version = rdr.read_u32::<NetworkEndian>().unwrap();
        if version != 1 {
            anyhow::bail!("unrecognized envelope version {}", version);
        }

        Ok(Envelope {
            nonce: &buf[4..4 + aead::NONCE_LEN],
            payload: &buf[4 + aead::NONCE_LEN..],
        })
    }

    fn to_bytes(&self) -> Vec<u8> {
        let mut buf = Vec::with_capacity(4 + self.nonce.len() + self.payload.len());

        buf.write_u32::<NetworkEndian>(ENVELOPE_VERSION).unwrap();
        buf.extend_from_slice(self.nonce);
        buf.extend_from_slice(self.payload);
        buf
    }
}

/// A unsealed payload with an attached version_id.  The version_id is used to
/// validate the context of the payload on unsealing.
pub(super) struct Unsealed {
    pub(super) version_id: Uuid,
    pub(super) payload: Vec<u8>,
}

/// An encrypted payload
pub(super) struct Sealed {
    pub(super) version_id: Uuid,
    pub(super) payload: Vec<u8>,
}

impl Sealed {
    pub(super) fn from_resp(
        resp: ureq::Response,
        version_id: Uuid,
        content_type: &str,
    ) -> Result<Sealed, anyhow::Error> {
        if resp.header("Content-Type") == Some(content_type) {
            let mut reader = resp.into_reader();
            let mut payload = vec![];
            reader.read_to_end(&mut payload)?;
            Ok(Self {
                version_id,
                payload,
            })
        } else {
            Err(anyhow::anyhow!(
                "Response did not have expected content-type"
            ))
        }
    }
}

impl AsRef<[u8]> for Sealed {
    fn as_ref(&self) -> &[u8] {
        self.payload.as_ref()
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn envelope_round_trip() {
        let env = Envelope {
            nonce: &[2; 12],
            payload: b"HELLO",
        };

        let bytes = env.to_bytes();
        let env2 = Envelope::from_bytes(&bytes).unwrap();
        assert_eq!(env, env2);
    }

    #[test]
    fn round_trip() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();

        let secret = Secret(b"SEKRIT".to_vec());
        let cryptor = Cryptor::new(Uuid::new_v4(), &secret).unwrap();

        let unsealed = Unsealed {
            version_id,
            payload: payload.clone(),
        };
        let sealed = cryptor.seal(unsealed).unwrap();
        let unsealed = cryptor.unseal(sealed).unwrap();

        assert_eq!(unsealed.payload, payload);
        assert_eq!(unsealed.version_id, version_id);
    }

    #[test]
    fn round_trip_bad_key() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let client_key = Uuid::new_v4();

        let secret = Secret(b"SEKRIT".to_vec());
        let cryptor = Cryptor::new(client_key, &secret).unwrap();

        let unsealed = Unsealed {
            version_id,
            payload: payload.clone(),
        };
        let sealed = cryptor.seal(unsealed).unwrap();

        let secret = Secret(b"DIFFERENT_SECRET".to_vec());
        let cryptor = Cryptor::new(client_key, &secret).unwrap();
        assert!(cryptor.unseal(sealed).is_err());
    }

    #[test]
    fn round_trip_bad_version() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let client_key = Uuid::new_v4();

        let secret = Secret(b"SEKRIT".to_vec());
        let cryptor = Cryptor::new(client_key, &secret).unwrap();

        let unsealed = Unsealed {
            version_id,
            payload: payload.clone(),
        };
        let mut sealed = cryptor.seal(unsealed).unwrap();
        sealed.version_id = Uuid::new_v4(); // change the version_id
        assert!(cryptor.unseal(sealed).is_err());
    }

    #[test]
    fn round_trip_bad_client_key() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let client_key = Uuid::new_v4();

        let secret = Secret(b"SEKRIT".to_vec());
        let cryptor = Cryptor::new(client_key, &secret).unwrap();

        let unsealed = Unsealed {
            version_id,
            payload: payload.clone(),
        };
        let sealed = cryptor.seal(unsealed).unwrap();

        let client_key = Uuid::new_v4();
        let cryptor = Cryptor::new(client_key, &secret).unwrap();
        assert!(cryptor.unseal(sealed).is_err());
    }
}

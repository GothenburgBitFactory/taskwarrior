use crate::server::HistorySegment;
use std::io::Read;
use tindercrypt::cryptors::RingCryptor;
use uuid::Uuid;

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

/// A cleartext payload with an attached version_id.  The version_id is used to
/// validate the context of the payload.
pub(super) struct Cleartext {
    pub(super) version_id: Uuid,
    pub(super) payload: HistorySegment,
}

impl Cleartext {
    /// Seal the payload into its ciphertext
    pub(super) fn seal(self, secret: &Secret) -> anyhow::Result<Ciphertext> {
        let cryptor = RingCryptor::new().with_aad(self.version_id.as_bytes());
        let ciphertext = cryptor.seal_with_passphrase(secret.as_ref(), &self.payload)?;
        Ok(Ciphertext(ciphertext))
    }
}

/// An ecrypted payload
pub(super) struct Ciphertext(pub(super) Vec<u8>);

impl Ciphertext {
    pub(super) fn from_resp(
        resp: ureq::Response,
        content_type: &str,
    ) -> Result<Ciphertext, anyhow::Error> {
        if resp.header("Content-Type") == Some(content_type) {
            let mut reader = resp.into_reader();
            let mut bytes = vec![];
            reader.read_to_end(&mut bytes)?;
            Ok(Self(bytes))
        } else {
            Err(anyhow::anyhow!(
                "Response did not have expected content-type"
            ))
        }
    }

    pub(super) fn open(self, secret: &Secret, version_id: Uuid) -> anyhow::Result<Cleartext> {
        let cryptor = RingCryptor::new().with_aad(version_id.as_bytes());
        let plaintext = cryptor.open(secret.as_ref(), &self.0)?;

        Ok(Cleartext {
            version_id,
            payload: plaintext,
        })
    }
}

impl AsRef<[u8]> for Ciphertext {
    fn as_ref(&self) -> &[u8] {
        self.0.as_ref()
    }
}

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn round_trip() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let cleartext = Cleartext {
            version_id,
            payload: payload.clone(),
        };
        let ciphertext = cleartext.seal(&secret).unwrap();
        let cleartext = ciphertext.open(&secret, version_id).unwrap();

        assert_eq!(cleartext.payload, payload);
        assert_eq!(cleartext.version_id, version_id);
    }

    #[test]
    fn round_trip_bad_key() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let cleartext = Cleartext {
            version_id,
            payload: payload.clone(),
        };
        let ciphertext = cleartext.seal(&secret).unwrap();

        let secret = Secret(b"BADSEKRIT".to_vec());
        assert!(ciphertext.open(&secret, version_id).is_err());
    }

    #[test]
    fn round_trip_bad_version() {
        let version_id = Uuid::new_v4();
        let payload = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let cleartext = Cleartext {
            version_id,
            payload: payload.clone(),
        };
        let ciphertext = cleartext.seal(&secret).unwrap();

        let bad_version_id = Uuid::new_v4();
        assert!(ciphertext.open(&secret, bad_version_id).is_err());
    }
}

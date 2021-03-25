use crate::server::HistorySegment;
use std::convert::TryFrom;
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

/// A cleartext payload containing a history segment.
pub(super) struct HistoryCleartext {
    pub(super) parent_version_id: Uuid,
    pub(super) history_segment: HistorySegment,
}

impl HistoryCleartext {
    /// Seal the payload into its ciphertext
    pub(super) fn seal(self, secret: &Secret) -> anyhow::Result<HistoryCiphertext> {
        let cryptor = RingCryptor::new().with_aad(self.parent_version_id.as_bytes());
        let ciphertext = cryptor.seal_with_passphrase(secret.as_ref(), &self.history_segment)?;
        Ok(HistoryCiphertext(ciphertext))
    }
}

/// An ecrypted payload containing a history segment
pub(super) struct HistoryCiphertext(pub(super) Vec<u8>);

impl HistoryCiphertext {
    pub(super) fn open(
        self,
        secret: &Secret,
        parent_version_id: Uuid,
    ) -> anyhow::Result<HistoryCleartext> {
        let cryptor = RingCryptor::new().with_aad(parent_version_id.as_bytes());
        let plaintext = cryptor.open(secret.as_ref(), &self.0)?;

        Ok(HistoryCleartext {
            parent_version_id,
            history_segment: plaintext,
        })
    }
}

impl TryFrom<ureq::Response> for HistoryCiphertext {
    type Error = anyhow::Error;

    fn try_from(resp: ureq::Response) -> Result<HistoryCiphertext, anyhow::Error> {
        if let Some("application/vnd.taskchampion.history-segment") = resp.header("Content-Type") {
            let mut reader = resp.into_reader();
            let mut bytes = vec![];
            reader.read_to_end(&mut bytes)?;
            Ok(Self(bytes))
        } else {
            Err(anyhow::anyhow!("Response did not have expected content-type"))
        }
    }
}

impl AsRef<[u8]> for HistoryCiphertext {
    fn as_ref(&self) -> &[u8] {
        self.0.as_ref()
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn round_trip() {
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let history_cleartext = HistoryCleartext {
            parent_version_id,
            history_segment: history_segment.clone(),
        };
        let history_ciphertext = history_cleartext.seal(&secret).unwrap();
        let history_cleartext = history_ciphertext.open(&secret, parent_version_id).unwrap();

        assert_eq!(history_cleartext.history_segment, history_segment);
        assert_eq!(history_cleartext.parent_version_id, parent_version_id);
    }

    #[test]
    fn round_trip_bad_key() {
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let history_cleartext = HistoryCleartext {
            parent_version_id,
            history_segment: history_segment.clone(),
        };
        let history_ciphertext = history_cleartext.seal(&secret).unwrap();

        let secret = Secret(b"BADSEKRIT".to_vec());
        assert!(history_ciphertext.open(&secret, parent_version_id).is_err());
    }

    #[test]
    fn round_trip_bad_pvid() {
        let parent_version_id = Uuid::new_v4();
        let history_segment = b"HISTORY REPEATS ITSELF".to_vec();
        let secret = Secret(b"SEKRIT".to_vec());

        let history_cleartext = HistoryCleartext {
            parent_version_id,
            history_segment: history_segment.clone(),
        };
        let history_ciphertext = history_cleartext.seal(&secret).unwrap();

        let bad_parent_version_id = Uuid::new_v4();
        assert!(history_ciphertext
            .open(&secret, bad_parent_version_id)
            .is_err());
    }
}

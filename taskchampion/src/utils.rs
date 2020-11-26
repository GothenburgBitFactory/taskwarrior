use std::convert::TryInto;
use uuid::Uuid;

/// A representation of a UUID as a key.  This is just a newtype wrapping the 128-bit packed form
/// of a UUID.
#[derive(Debug, PartialEq, Eq, PartialOrd, Ord)]
pub(crate) struct Key(uuid::Bytes);

impl From<&[u8]> for Key {
    fn from(bytes: &[u8]) -> Key {
        Key(bytes.try_into().unwrap())
    }
}

impl From<&Uuid> for Key {
    fn from(uuid: &Uuid) -> Key {
        let key = Key(*uuid.as_bytes());
        key
    }
}

impl From<Uuid> for Key {
    fn from(uuid: Uuid) -> Key {
        let key = Key(*uuid.as_bytes());
        key
    }
}

impl From<Key> for Uuid {
    fn from(key: Key) -> Uuid {
        Uuid::from_bytes(key.0)
    }
}

impl AsRef<[u8]> for Key {
    fn as_ref(&self) -> &[u8] {
        &self.0[..]
    }
}

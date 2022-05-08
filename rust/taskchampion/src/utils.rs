use std::convert::TryInto;
use uuid::Uuid;

/// A representation of a UUID as a key.  This is just a newtype wrapping the 128-bit packed form
/// of a UUID.
#[derive(Debug, PartialEq, Eq, PartialOrd, Ord)]
pub(crate) struct Key(uuid::Bytes);

impl From<&[u8]> for Key {
    fn from(bytes: &[u8]) -> Key {
        Key(bytes.try_into().expect("expected 16 bytes"))
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

#[cfg(test)]
mod test {
    use super::*;
    use pretty_assertions::assert_eq;

    #[test]
    fn test_from_bytes() {
        let k: Key = (&[1u8, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16][..]).into();
        let u: Uuid = k.into();
        assert_eq!(
            u,
            Uuid::parse_str("01020304-0506-0708-090a-0b0c0d0e0f10").unwrap()
        );
    }

    #[test]
    #[should_panic]
    fn test_from_bytes_bad_len() {
        let _: Key = (&[1u8, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11][..]).into();
    }
}

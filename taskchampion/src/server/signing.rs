#![allow(dead_code)] // TODO: temporary until this module is used
//! This is a general wrapper around an asymmetric-key signature system.

use failure::Fallible;
use ring::{
    rand,
    signature::{Ed25519KeyPair, KeyPair, Signature, UnparsedPublicKey, ED25519},
};

type PublicKey = Vec<u8>;
type PrivateKey = Vec<u8>;

/// Generate a pair of (public, private) key material (in fact the private key is a keypair)
pub fn new_keypair() -> Fallible<(PublicKey, PrivateKey)> {
    let rng = rand::SystemRandom::new();
    let key_pkcs8 = Ed25519KeyPair::generate_pkcs8(&rng)?;
    let key_pair = Ed25519KeyPair::from_pkcs8(key_pkcs8.as_ref())?;
    let pub_key = key_pair.public_key();
    Ok((
        pub_key.as_ref().to_vec() as PublicKey,
        key_pkcs8.as_ref().to_vec() as PrivateKey,
    ))
}

pub struct Signer {
    key_pair: Ed25519KeyPair,
}

impl Signer {
    /// Create a new signer, given a pkcs#8 v2 document containing the keypair.
    fn new(priv_key: PrivateKey) -> Fallible<Self> {
        Ok(Self {
            key_pair: Ed25519KeyPair::from_pkcs8(&priv_key)?,
        })
    }

    pub fn sign<B: AsRef<[u8]>>(&self, message: B) -> Fallible<Signature> {
        Ok(self.key_pair.sign(message.as_ref()))
    }
}

pub struct Verifier {
    pub_key: PublicKey,
}

impl Verifier {
    fn new(pub_key: PublicKey) -> Fallible<Self> {
        Ok(Self { pub_key })
    }

    pub fn verify<B1: AsRef<[u8]>, B2: AsRef<[u8]>>(
        &self,
        message: B1,
        signature: B2,
    ) -> Fallible<()> {
        let pub_key = UnparsedPublicKey::new(&ED25519, &self.pub_key);
        Ok(pub_key.verify(message.as_ref(), signature.as_ref())?)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn test_verify_ok() -> Fallible<()> {
        let (public, private) = new_keypair()?;
        let signer = Signer::new(private)?;
        let verifier = Verifier::new(public)?;

        let message = b"Hello, world";
        let signature = signer.sign(message)?;
        verifier.verify(message, signature)
    }

    #[test]
    fn test_verify_bad_message() -> Fallible<()> {
        let (public, private) = new_keypair()?;
        let signer = Signer::new(private)?;
        let verifier = Verifier::new(public)?;

        let message = b"Hello, world";
        let signature = signer.sign(message)?;
        assert!(verifier.verify(b"Hello, cruel world", signature).is_err());
        Ok(())
    }
}

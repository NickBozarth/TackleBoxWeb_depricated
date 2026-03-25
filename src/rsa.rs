use std::sync::OnceLock;

use axum::body::Bytes;
use base64::{Engine, prelude::BASE64_STANDARD};
use rsa::{
    Oaep, RsaPrivateKey, RsaPublicKey,
    pkcs8::{DecodePublicKey, EncodePublicKey, LineEnding},
};

use anyhow::{Result, anyhow};
use sha2::Sha256;

static PRIVATE_KEY: OnceLock<RsaPrivateKey> = OnceLock::new();
static PUBLIC_KEY: OnceLock<RsaPublicKey> = OnceLock::new();
static PUBLIC_KEY_STRING: OnceLock<String> = OnceLock::new();

pub fn gen_keys() -> Result<()> {
    let mut rng = rand::thread_rng();
    const BITCOUNT: usize = 2048;

    PRIVATE_KEY
        .set(
            RsaPrivateKey::new(&mut rng, BITCOUNT)
                .map_err(|_| anyhow!("Failed to generate PRIVATE_KEY"))?,
        )
        .map_err(|_| anyhow!("Failed to set PRIVATE_KEY"))?;
    PUBLIC_KEY
        .set(PRIVATE_KEY.get().unwrap().to_public_key())
        .map_err(|_| anyhow!("Failed to set PUBLIC_KEY"))?;
    PUBLIC_KEY_STRING
        .set(
            PUBLIC_KEY
                .get()
                .unwrap()
                .to_public_key_pem(LineEnding::LF)
                .map_err(|_| anyhow!("Failed to pem PUBLIC_KEY"))?
                .replace("\n", ""),
        )
        .map_err(|_| anyhow!("Failed to set PUBLIC_KEY_STRING"))?;

    Ok(())
}

pub fn get_pubkey() -> &'static String {
    PUBLIC_KEY_STRING
        .get()
        .expect("Failed to get PUBLIC_KEY_STRING")
}


pub fn import_pubkey(pubkey_str_b64: &str) -> Result<RsaPublicKey> {
    let pubkey_str = BASE64_STANDARD
        .decode(pubkey_str_b64)
        .map_err(|_| anyhow!("Failed to decode pubkey_str_b64"))?;

    RsaPublicKey::from_public_key_der(&pubkey_str)
        .map_err(|_| anyhow!("Failed to import key"))
}


pub trait Cryptography {
    fn into_encrypted(self, client_pubkey: RsaPublicKey) -> Result<Vec<u8>, rsa::Error>;
    fn into_decrypted(self) -> Result<Vec<u8>, rsa::Error>;

    fn into_b64_encoded(self) -> String;
    fn into_b64_decoded(self) -> Result<Vec<u8>, base64::DecodeError>;
}

impl<T: AsRef<[u8]>> Cryptography for T {
    fn into_encrypted(self, client_pubkey: RsaPublicKey) -> Result<Vec<u8>, rsa::Error> {
        let padding = Oaep::new_with_mgf_hash::<Sha256, Sha256>();
        let mut rng = rand::thread_rng();
        client_pubkey.encrypt(&mut rng, padding, self.as_ref())
    }

    fn into_decrypted(self) -> Result<Vec<u8>, rsa::Error> {
        let padding = Oaep::new_with_mgf_hash::<Sha256, Sha256>();
        PRIVATE_KEY
            .get()
            .expect("Failed to retrieve PRIVATE_KEY")
            .decrypt(padding, self.as_ref())
    }

    fn into_b64_encoded(self) -> String {
        BASE64_STANDARD.encode(self)
    }

    fn into_b64_decoded(self) -> Result<Vec<u8>, base64::DecodeError> {
        BASE64_STANDARD.decode(self)
    }
}
use std::sync::OnceLock;

use axum::body::Bytes;
use rsa::{
    Oaep, RsaPrivateKey, RsaPublicKey,
    pkcs8::{EncodePublicKey, LineEnding},
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

pub trait Cryptography {
    fn into_encrypted(self) -> Bytes;
    fn into_decrypted(self) -> Result<Vec<u8>, rsa::Error>;
}

impl Cryptography for Vec<u8> {
    fn into_encrypted(self) -> Bytes {
        todo!()
    }

    fn into_decrypted(self) -> Result<Vec<u8>, rsa::Error> {
        let padding = Oaep::new_with_mgf_hash::<Sha256, Sha256>();
        PRIVATE_KEY
            .get()
            .expect("Failed to retrieve PRIVATE_KEY")
            .decrypt(padding, &self)
    }
}
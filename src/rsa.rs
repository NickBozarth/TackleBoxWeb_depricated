use std::{ffi::os_str::Display, marker::PhantomData, str::Bytes, sync::OnceLock};

use base64::{Engine, prelude::BASE64_STANDARD};
use rsa::{
    Oaep, RsaPrivateKey, RsaPublicKey,
    pkcs8::{DecodePublicKey, EncodePublicKey, LineEnding},
};

use anyhow::{Result, anyhow};
use serde::Deserialize;
use sha2::{Digest, Sha256};

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


#[derive(Debug, Deserialize)]
#[serde(transparent)]
pub struct Base64String<Inner>(String, PhantomData<Inner>);

impl<Inner: From<Vec<u8>>> Base64String<Inner> {
    pub fn into_b64_decoded(self) -> Result<Inner, base64::DecodeError> {
        BASE64_STANDARD.decode(self.0)
            .map(|decoded_buffer| decoded_buffer.into())
    }
}

impl<Inner> From<Vec<u8>> for Base64String<Inner> {
    fn from(value: Vec<u8>) -> Self {
        Self (
            String::from_utf8_lossy(&value).to_string(),
            PhantomData
        )
    }
}

impl<Inner> ToString for Base64String<Inner> {
    fn to_string(&self) -> String {
        self.0.clone()
    }
}

impl<Inner> From<String> for Base64String<Inner> {
    fn from(value: String) -> Self {
        Self (
            value, 
            PhantomData
        )
    }
}

impl<T> PartialEq for Base64String<T> {
    fn eq(&self, other: &Self) -> bool {
        self.0 == other.0
    }
}



#[derive(Clone)]
pub struct RsaEncrypted<T>(Vec<u8>, PhantomData<T>);

impl<T: From<Vec<u8>>> RsaEncrypted<T> {

    pub fn into_rsa_decrypted(self, priv_key: &RsaPrivateKey) -> Result<T, rsa::Error> {
        let padding = Oaep::new_with_mgf_hash::<Sha256, Sha256>();

        priv_key
            .decrypt(padding, &self.0)
            .map(|decrypted_buffer| decrypted_buffer.into())
    }

    pub fn into_rsa_decrypted_global(self) -> Result<T, rsa::Error> {
        let global_priv_key = PRIVATE_KEY
            .get()
            .expect("Failed to retrieve PRIVATE_KEY");
        
        self.into_rsa_decrypted(global_priv_key)
    }
}

impl<T> From<Vec<u8>> for RsaEncrypted<T> {
    fn from(value: Vec<u8>) -> Self {
        Self (
            value,
            PhantomData
        )
    }
}

impl<T> AsRef<[u8]> for RsaEncrypted<T> {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}




#[derive(Debug)]
pub struct Hash<Alg> (Vec<u8>, PhantomData<Alg>);

impl<Alg: Digest> Hash<Alg> {
    pub fn digest_buffer(buffer: &[u8]) -> Self {
        Self (
            Alg::digest(buffer)
                .into_iter()
                .collect(),
            PhantomData
        )
    }
}

impl<Alg> AsRef<[u8]> for Hash<Alg> {
    fn as_ref(&self) -> &[u8] {
        self.0.as_ref()
    }
}


pub trait Cryptography: Sized {
    fn interpret_as_rsa_encrypted(&self) -> RsaEncrypted<Self>;
    fn into_rsa_encrypted(self, client_pubkey: RsaPublicKey) -> Result<RsaEncrypted<Self>, rsa::Error>;

    fn interpret_as_b64_string<Inner: From<Vec<u8>>>(self) -> Base64String<Inner>;
    fn into_b64_encoded(self) -> Base64String<Self>;

    fn get_hash<Alg: Digest>(&self) -> Hash<Alg>;
}

impl<T: Sized + AsRef<[u8]>> Cryptography for T {
    fn interpret_as_rsa_encrypted(&self) -> RsaEncrypted<Self> {
        RsaEncrypted(
            self.as_ref().to_vec(),
            PhantomData 
        )
    }


    fn into_rsa_encrypted(self, client_pubkey: RsaPublicKey) -> Result<RsaEncrypted<Self>, rsa::Error> {
        let padding = Oaep::new_with_mgf_hash::<Sha256, Sha256>();
        let mut rng = rand::thread_rng();

        client_pubkey.encrypt(&mut rng, padding, self.as_ref())
            .map(|bytes| RsaEncrypted(bytes, PhantomData::<Self>))
    }


    fn interpret_as_b64_string<Inner: From<Vec<u8>>>(self) -> Base64String<Inner> {
        Base64String(
            String::from_utf8_lossy(self.as_ref()).to_string(), 
            PhantomData
        )
    }



    fn into_b64_encoded(self) -> Base64String<Self> {
        Base64String(
            BASE64_STANDARD.encode(self), 
            PhantomData
        )
    }


    // fn get_b64_decoded(&self) -> Result<Vec<u8>, base64::DecodeError> {
    //     let x = String::from("what");

    //     BASE64_STANDARD.decode(self)
    // }

    fn get_hash<Alg: Digest>(&self) -> Hash<Alg> {
        Hash::digest_buffer(self.as_ref())
    }
}
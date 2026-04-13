use aes_gcm::Aes256Gcm;
use serde::Deserialize;

use crate::{get_endpoints::TackleType, rsa::{AesEncrypted, Base64String}, sessions::SessionId};



#[derive(Debug, Deserialize)]
pub struct NewTackleRequest {
    session_id: SessionId,
    iv: Vec<u8>,

    name: Base64String<AesEncrypted<Aes256Gcm, String>>,
    description: Base64String<AesEncrypted<Aes256Gcm, String>>,
    tackle_type: TackleType,

    text_data: Option<Base64String<AesEncrypted<Aes256Gcm, String>>>,
    file_data: Option<Base64String<AesEncrypted<Aes256Gcm, String>>>,

    share_with: Option<Base64String<AesEncrypted<Aes256Gcm, Vec<String>>>>
}
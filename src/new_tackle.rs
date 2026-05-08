use serde::Deserialize;

use crate::{get_endpoints::TackleType, rsa::{AesEncrypted, Base64String}, sessions::SessionId};



#[derive(Debug, Deserialize)]
pub struct NewTackleRequest {
    session_id: SessionId,
    #[serde(rename = "iv")]
    nonce: Vec<u8>,

    name: Base64String<AesEncrypted<String>>,
    tackle_type: TackleType,
    description: Option<Base64String<AesEncrypted<String>>>,

    text_data: Option<Base64String<AesEncrypted<String>>>,
    file_data: Option<Base64String<AesEncrypted<String>>>,

    share_with: Option<Base64String<AesEncrypted<Vec<String>>>> // TODO
}
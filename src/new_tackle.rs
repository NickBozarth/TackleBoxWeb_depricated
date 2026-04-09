use serde::Deserialize;

use crate::sessions::SessionId;



#[derive(Debug, Deserialize)]
pub struct NewTackleRequest {
    session_id: SessionId,

    name: String,
    description: Option<String>,

    text_data: Option<String>,
    file_data: Option<String>,

    iv: Vec<u8>
}
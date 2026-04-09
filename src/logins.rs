
use std::{collections::HashMap, marker::PhantomData, sync::OnceLock};

use serde::Deserialize;
use sha2::Sha256;
use sqlx::{Row, prelude::FromRow, sqlite::SqliteRow};
use tokio::sync::Mutex;

use crate::rsa::{Base64String, Hash};

#[derive(Deserialize)]
pub struct SignupHash {
    pub username_hash: Base64String<Hash<Sha256>>,
    pub login_hash: Base64String<Hash<Sha256>>,
}

#[derive(Debug, Deserialize)]
pub struct LoginHash {
    pub username_hash: Base64String<Hash<Sha256>>,
    pub login_hash2: Option<Base64String<Hash<Sha256>>>, // login_hash + salt
    pub request_id: Option<u32>
}

#[derive(Debug)]
pub struct LoginRequest {
    pub id: u32,
    pub username_hash: Base64String<Hash<Sha256>>,
    pub salt: Base64String<[u8; 12]>,
}

#[derive(Debug)]
pub struct Login {
    pub username_hash: Base64String<Hash<Sha256>>,
    pub login_hash: Base64String<Hash<Sha256>>
}

impl FromRow<'_, SqliteRow> for Login {
    fn from_row(row: &SqliteRow) -> Result<Self, sqlx::Error> {
        let username_string: String  = row.try_get("username_hash")?;
        let login_string: String = row.try_get("login_hash")?;

        Ok(
            Self {
                username_hash: username_string.into(),
                login_hash: login_string.into()
            }
        )
    }
}


pub const MAX_LOGIN_REQUESTS: usize = 10;
static LOGIN_REQUESTS_MAP: OnceLock<Mutex<HashMap<u32, LoginRequest>>> = OnceLock::new();
pub fn get_login_requests_map() -> &'static Mutex<HashMap<u32, LoginRequest>> {
    LOGIN_REQUESTS_MAP.get_or_init(|| Mutex::new(HashMap::new()))
}
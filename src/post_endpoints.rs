use std::{collections::HashMap, sync::OnceLock};

use axum::{Json, http::StatusCode};
use serde::{Deserialize, Serialize};
use sqlx::{Executor, error::DatabaseError, sqlite::SqliteError};
use tokio::sync::Mutex;

use crate::extractors::Pool;

#[derive(Debug, Deserialize)]
pub struct SignupHash {
    username_hash: String,
    login_hash: String,
}

#[derive(Serialize)]
pub struct LoginRequest {
    id: i32,
    username_hash: String,
    iv: String,
}

static LOGIN_REQUESTS_MAP: OnceLock<Mutex<HashMap<i32, LoginRequest>>> = OnceLock::new();
fn get_login_requests_map() -> &'static Mutex<HashMap<i32, LoginRequest>> {
    LOGIN_REQUESTS_MAP.get_or_init(|| Mutex::new(HashMap::new()))
}

pub async fn signup_endpoint(
    Pool(pool): Pool<'_>,
    Json(signup_hash): Json<SignupHash>,
) -> &'static str {
    // attempt to insert signup
    let query_result = sqlx::query("INSERT INTO logins VALUES (?, ?)")
        .bind(signup_hash.username_hash)
        .bind(signup_hash.login_hash)
        .execute(pool)
        .await;

    
    // parse result of sql query
    if let Ok(result) = &query_result && result.rows_affected() == 1 {
        return "Success";
    }
    if let Err(sqlx::Error::Database(db_err)) = &query_result {
        let code = db_err.downcast_ref::<SqliteError>().code();
        if code.as_deref() == Some("1555") { return "User already exists" }
    }
    "Database error"
}

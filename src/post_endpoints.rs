use axum::{Json, http::StatusCode};
use serde::Deserialize;

#[derive(Debug, Deserialize)]
pub struct LoginHash {
    username:   String,
    login_hash: Option<String>
}


pub async fn signup_endpoint(Json(login_hash): Json<LoginHash>) -> StatusCode {
    println!("Payload {:?}", login_hash);
    StatusCode::OK
}
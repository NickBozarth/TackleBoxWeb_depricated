use axum::{extract::FromRequestParts, http::{StatusCode, request::Parts}};
use sqlx::SqlitePool;

use crate::app_state::AppState;


pub struct Pool<'a>(pub &'a SqlitePool);

impl<'a> FromRequestParts<AppState<'a>> for Pool<'a> {
    type Rejection = StatusCode;

    async fn from_request_parts(_parts: &mut Parts, state: &AppState<'a>) -> Result<Self, Self::Rejection> {
        Ok(Pool(state.pool))
    }
}
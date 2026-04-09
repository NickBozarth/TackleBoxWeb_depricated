use std::{collections::HashMap, sync::OnceLock};

use axum::{Json, extract::{FromRequest, Request, State}, http::StatusCode, response::IntoResponse};
use axum_extra::extract::{CookieJar, cookie::Cookie};
use serde::{Deserialize, Serialize};
use sqlx::{Executor, error::DatabaseError, FromRow, sqlite::SqliteError};
use tokio::sync::Mutex;

use crate::{app_state::AppState, extractors::Pool, rsa::Cryptography, sessions::{Session, SessionId}};

#[derive(Deserialize)]
pub struct SignupHash {
    username_hash: String,
    login_hash: String,
}

#[derive(Debug, Deserialize)]
pub struct LoginHash {
    username_hash: String,
    pub login_hash2: Option<String>, // login_hash + salt
    request_id: Option<u32>
}

#[derive(Debug)]
pub struct LoginRequest {
    id: u32,
    username_hash: String,
    salt: String,
}

#[derive(Debug, FromRow)]
struct Login {
    username_hash: String,
    login_hash: String
}


const MAX_LOGIN_REQUESTS: usize = 10;
static LOGIN_REQUESTS_MAP: OnceLock<Mutex<HashMap<u32, LoginRequest>>> = OnceLock::new();
fn get_login_requests_map() -> &'static Mutex<HashMap<u32, LoginRequest>> {
    LOGIN_REQUESTS_MAP.get_or_init(|| Mutex::new(HashMap::new()))
}


// TODO make signup an toggleable feature
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
    else if let Err(sqlx::Error::Database(db_err)) = &query_result {
        let code = db_err.downcast_ref::<SqliteError>().code();
        if code.as_deref() == Some("1555") { 
            return "User already exists"; 
        }
    }

    return "Database error";
}


// login has 2 paths, one where login_hash is Some and the other where it is None
// login_hash == None => return the LoginRequest.id and salt to hash with the login_hash 
//   on both ends so the login hash always changes 
// login_hash == Some(_) => return "Success" or reason for failiure 
pub async fn login_endpoint(Pool(pool): Pool<'_>, Json(login_hash): Json<LoginHash>) -> Result<String, String> {
    let mut login_request_map = get_login_requests_map()
        .lock()
        .await;

    
    // initial request goes here to retrieve requestid and salt
    if login_hash.request_id.is_none() || login_hash.login_hash2.is_none() {
        if login_request_map.len() >= MAX_LOGIN_REQUESTS {
            return Err("Server is processing too many login requests. Please try again later".to_owned());
        }

        // ensure that user is contained in db
        sqlx::query("SELECT 1 FROM logins WHERE username_hash = ?")
            .bind(&login_hash.username_hash)
            .execute(pool)
            .await
            .map_err(|_| "Username not found".to_owned())?;

        // create a guaranteed unique request_id
        let mut request_id: u32;
        loop {
            request_id = rand::random();
            if !login_request_map.contains_key(&request_id) { break; }
        }

        // generate random base64 to salt hash with
        let salt: [u8;12] = rand::random();
        let saltb64 = salt.into_b64_encoded();
        let ret = format!("{{ \"request_id\": {}, \"salt\": \"{}\"}}", request_id, saltb64);

        // create new request for server to hold onto
        let login_request = LoginRequest { 
            id: request_id, 
            username_hash: login_hash.username_hash, 
            salt: saltb64
        };
        login_request_map.insert(request_id, login_request);

        return Ok(ret);
    }





    // find the login request that corresponds with the request_id provided
    let login_request = login_request_map
        .remove(&login_hash.request_id.unwrap())
        .ok_or("No login request found with provided request_id".to_string())?;


    // find the login from the database that corresponds to the given login_hash.login_hash
    let internal_login_hash: String = sqlx::query_as::<_, Login>(
        "
            SELECT * 
            FROM logins 
            WHERE username_hash = ?
        "
    )
        .bind(login_hash.username_hash)
        .fetch_one(pool)
        .await
        .map_err(|_| "Incorrect username or password".to_owned())?
        .login_hash;

    let internal_login_hash2 = (internal_login_hash + &login_request.salt)
        .into_sha256()
        .into_b64_encoded();


    
    println!("MADE IT HERER");
    // login_hash.login_hash2 is Some(_)
    let resp = match internal_login_hash2 == login_hash.login_hash2.unwrap() {
        true => {
            // create session
            let session = Session::new();
            let session_id_u32 = session.id.0;
            Ok(format!("{{ \"session_id\": {} }}", session_id_u32))
        },
        false => Err("Incorrect username or password".to_owned())
    };

    println!("RESPONSE {:#?}", resp);
    resp
}


pub async fn check_session(Json(session): Json<Session>) -> StatusCode {
    match session.is_valid() {
        true => StatusCode::OK,
        false => StatusCode::BAD_REQUEST
    }
}




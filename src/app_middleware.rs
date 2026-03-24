use crate::rsa::{Cryptography, get_pubkey};
use axum::body::{Body, HttpBody};
use axum::http::Method;
use axum::middleware::Next;
use axum::response::IntoResponse;
use axum::{extract::Request, response::Response};
use axum_extra::extract::{CookieJar, cookie::Cookie};
use base64::Engine as _;
use base64::prelude::BASE64_STANDARD;
use http_body_util::BodyExt;

pub async fn key_manager(request: Request, next: Next) -> Response {
    // get the server-public key cookie
    let headers = Request::headers(&request);
    let mut cookies = CookieJar::from_headers(headers);
    let pubkey_cookie = cookies.get("server-public-key");

    let response = next.run(request).await;

    // add pubkey if it is unset or does not equal the current pubkey
    let current_pubkey = get_pubkey();
    if pubkey_cookie.map(|cookie| cookie.value()) != Some(current_pubkey) {
        cookies = cookies.add(Cookie::new("server-public-key", get_pubkey()));
    }

    (cookies, response).into_response()
}

pub async fn encryption_manager(request: Request, next: Next) -> Response {
    // only act on POST requests
    if request.method() != Method::POST {
        let resp = next.run(request).await;
        return resp;
    }

    // retrieve encrypted body
    let (parts, encrypted_body) = request.into_parts();
    let encrypted_bytes_b64 = encrypted_body
        .collect()
        .await
        .expect("Failed to unwrap body")
        .to_bytes();


    let decrypted_body = BASE64_STANDARD
        .decode(encrypted_bytes_b64)
        .expect("Failed to decode b64")
        .into_decrypted()
        .expect("Failed to decrypt body");

    let new_body = Body::from(decrypted_body);
    let new_request = axum::http::Request::from_parts(parts, new_body);

    let resp = next.run(new_request).await;

    resp
}

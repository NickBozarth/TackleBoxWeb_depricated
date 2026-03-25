use crate::rsa::{Cryptography, import_pubkey, get_pubkey};
use axum::body::{Body, HttpBody};
use axum::http::Method;
use axum::middleware::Next;
use axum::response::IntoResponse;
use axum::{extract::Request, response::Response};
use axum_extra::extract::{CookieJar, cookie::Cookie};
use base64::Engine as _;
use base64::prelude::BASE64_STANDARD;
use http_body_util::BodyExt;
use rsa::pkcs8::EncodePublicKey;

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

pub async fn codec_manager(request: Request, next: Next) -> Response {
    // only act on POST requests
    if request.method() != Method::POST {
        let resp = next.run(request).await;
        return resp;
    }


    // make sure there is a client key present and import
    let cookies = CookieJar::from_headers(request.headers());
    let client_pubkey_cookie =  cookies
        .get("client-public-key")
        .expect("No client-public-key cookie present");
    let client_pubkey = import_pubkey(client_pubkey_cookie.value())
        .expect("Failed to import client pubkey");

    // retrieve encrypted body
    let (parts, encrypted_body) = request.into_parts();
    let encrypted_bytes_b64 = encrypted_body
        .collect()
        .await
        .expect("Failed to unwrap body")
        .to_bytes();


    // base64 decode and decrypt
    let decrypted_body = BASE64_STANDARD
        .decode(encrypted_bytes_b64)
        .expect("Failed to decode b64")
        .into_decrypted()
        .expect("Failed to decrypt body");

    let new_body = Body::from(decrypted_body);
    let new_request = axum::http::Request::from_parts(parts, new_body);



    let resp = next.run(new_request).await;


    // get the body from request
    let (parts, body) = resp.into_parts();


    // encrypt body
    let body_bytes = body
        .into_data_stream()
        .collect()
        .await
        .expect("Failed to unwrap body")
        .to_bytes()
        .into_encrypted(client_pubkey)
        .expect("Failed to encrypt body")
        .into_b64_encoded();

    println!("BODY BYTES {:?}", body_bytes);

    // construct and return response
    let resp_body = Body::from(body_bytes);
    let new_resp = Response::from_parts(parts, resp_body);

    new_resp
}
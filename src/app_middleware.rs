use crate::rsa::{Base64String, Cryptography, RsaEncrypted, get_pubkey, import_pubkey};
use axum::body::Body;
use axum::http::{Method, StatusCode};
use axum::middleware::Next;
use axum::response::IntoResponse;
use axum::{extract::Request, response::Response};
use axum_extra::extract::{CookieJar, cookie::Cookie};
use http_body_util::BodyExt;



fn request_is_encrypted(request: &Request) -> bool {
    request.method() == Method::POST
}


pub async fn key_manager(request: Request, next: Next) -> Response {
    // get the server-public key cookie
    let headers = Request::headers(&request);
    let mut cookies = CookieJar::from_headers(headers);
    let pubkey_cookie = cookies.get("server-public-key");

    // add pubkey if it is unset or does not equal the current pubkey
    let current_pubkey = get_pubkey();
    if pubkey_cookie.map(|cookie| cookie.value()) != Some(current_pubkey) {
        cookies = cookies.add(Cookie::new("server-public-key", get_pubkey()));

        if request_is_encrypted(&request) {
            return (cookies, StatusCode::PRECONDITION_FAILED).into_response();
        }
    }


    let response = next.run(request).await;
    

    (cookies, response).into_response()
}


pub async fn encryption_manager(request: Request, next: Next) -> Response {
    if !request_is_encrypted(&request) {
        let resp = next.run(request).await;
        return resp;
    }

    // make sure there is a client key present and import
    let cookies = CookieJar::from_headers(request.headers());
    let client_pubkey_cookie = cookies
        .get("client-public-key")
        .expect("No client-public-key cookie present");
    let client_pubkey = import_pubkey(client_pubkey_cookie.value()).expect("Failed to import client pubkey");

    // retrieve encrypted body
    let (parts, encrypted_body) = request.into_parts();

    // decrypt body
    let decrypted_body = encrypted_body 
        .collect()
        .await
        .expect("Failed to collect response body")
        //convert to bytes
        .to_bytes()
        // tell compiler the bytes are a base64 string
        .interpret_as_b64_string::<RsaEncrypted<Vec<u8>>>() 
        .into_b64_decoded()
        .expect("Failed to base64 decode body")
        .into_rsa_decrypted_global()
        .expect("Failed to decrypt body");



    println!("DECRYPTED BODY: {:?}", String::from_utf8_lossy(&decrypted_body));

    let new_body = Body::from(decrypted_body);
    let new_request = axum::http::Request::from_parts(parts, new_body);

    // run next step in the middleware stack
    let resp = next.run(new_request).await;

    // get the body from request
    let (parts, body) = resp.into_parts();

    // encrypt body
    let body_string: String = body
        .into_data_stream()
        .collect()
        .await
        .expect("Failed to unwrap body")
        .to_bytes()
        .into_rsa_encrypted(client_pubkey)
        .expect("Failed to encrypt body")
        .into_b64_encoded()
        .to_string();


    // construct and return response
    let resp_body = Body::from(body_string);
    let new_resp = Response::from_parts(parts, resp_body);

    new_resp
}

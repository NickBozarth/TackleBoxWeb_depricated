use axum::{
    extract::Request,
    response::Response,
};
use axum::middleware::Next;
use axum_extra::extract::{cookie::Cookie, CookieJar};


pub async fn my_middleware(request: Request, next: Next) -> Response {

    CookieJar::FromRequestParts(request);

    // PRE-PROCESSING: Do something before the handler runs
    println!("Request received: {} {}", request.method(), request.uri());

    let response = next.run(request).await;

    // POST-PROCESSING: Modify the response before it goes to the client
    println!("Response status: {}", response.status());

    response
}

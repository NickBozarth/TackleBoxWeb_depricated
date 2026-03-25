use axum::{
    Router, extract::DefaultBodyLimit, middleware, response::Json, routing::{
        get,
        get_service, post
    }
};
use serde_json::{
    Value, 
    json,
};
use tower_http::services::{
    ServeDir,
    ServeFile,
};
use anyhow::Result;

mod app_middleware;
use app_middleware::{key_manager, codec_manager};

mod rsa;
use rsa::gen_keys;

mod post_endpoints;
use post_endpoints::*;

// `&'static str` becomes a `200 OK` with `content-type: text/plain; charset=utf-8`
async fn plain_text() -> &'static str {
    "foo"
}

// `Json` gives a content-type of `application/json` and works with any type
// that implements `serde::Serialize`
async fn json() -> Json<Value> {
    Json(json!({ "data": 42 }))
}

#[tokio::main]
async fn main() -> Result<()> {
    gen_keys()?;

    let serve_dir = ServeDir::new("serve");
    let serve_index = ServeFile::new("serve/index.html");

    let app = Router::new()
        .route("/plain_text", get(plain_text))
        .route("/json", get(json))
        .route("/", get_service(serve_index))
        .route("/path", post(path_page))
        .fallback_service(serve_dir)
        .layer(DefaultBodyLimit::max(1024 * 1000)) // limit traffic to 100kb or other arbitrary number
        .layer(middleware::from_fn(key_manager))
        .layer(middleware::from_fn(codec_manager));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080").await.unwrap();
    println!("Listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
    Ok(())
}

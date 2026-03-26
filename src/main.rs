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


#[tokio::main]
async fn main() -> Result<()> {
    gen_keys()?;

    let serve_dir = ServeDir::new("serve");

    let app = Router::new()
        .route("/", get_service(ServeFile::new("serve/index.html")))
        .route("/signup", post(signup_endpoint))
        .fallback_service(serve_dir)
        .layer(DefaultBodyLimit::max(1024 * 1000)) // limit traffic to 100kb so i dont blow up codec manager
        .layer(middleware::from_fn(key_manager))
        .layer(middleware::from_fn(codec_manager));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080").await.unwrap();
    println!("Listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
    Ok(())
}

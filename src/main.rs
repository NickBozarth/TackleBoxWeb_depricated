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
use sqlx::SqlitePool;
use tower_http::services::{
    ServeDir,
    ServeFile,
};
use anyhow::Result;

mod app_middleware;
use app_middleware::{key_manager, encryption_manager};

mod rsa;
use rsa::gen_keys;

mod post_endpoints;
use post_endpoints::*;

mod extractors;

mod app_state;
use app_state::AppState;

mod sessions;




#[tokio::main]
async fn main() -> Result<()> {
    let state = AppState::new().await;
    gen_keys()?;


    let app = Router::new()
        .route("/", get_service(ServeFile::new("serve/index.html")))
        .route("/signup", post(signup_endpoint))
        .route("/login", post(login_endpoint))
        .fallback_service(ServeDir::new("serve"))
        .layer(DefaultBodyLimit::max(1024 * 1000)) // limit traffic to 100kb so i dont blow up codec manager
        .layer(middleware::from_fn(encryption_manager))
        .layer(middleware::from_fn(key_manager))
        .with_state(state);

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080").await.unwrap();
    println!("Listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
    Ok(())
}

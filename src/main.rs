use axum::{
    body::Body,
    routing::{
        get,
        get_service
    },
    response::Json,
    Router,
    middleware,
};
use serde_json::{
    Value, 
    json,
};
use tower_http::services::{
    ServeDir,
    ServeFile,
};

mod app_middleware;
use app_middleware::my_middleware;

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
async fn main() {
    let serve_dir = ServeDir::new("serve");
    let serve_index = ServeFile::new("serve/index.html");

    let app = Router::new()
        .route("/plain_text", get(plain_text))
        .route("/json", get(json))
        .route("/", get_service(serve_index))
        .fallback_service(serve_dir)
        .layer(middleware::from_fn(my_middleware));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080").await.unwrap();
    println!("Listening on {}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

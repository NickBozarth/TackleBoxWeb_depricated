use std::sync::OnceLock;

use axum::Json;
use serde::{Deserialize, Serialize};





#[derive(Serialize)]
pub struct TackleType(&'static str, &'static str);

static SECTIONS: OnceLock<Vec<TackleType>> = OnceLock::new();
fn get_global_tackle_types() -> &'static Vec<TackleType> {
    SECTIONS.get_or_init(|| {
        vec![
            TackleType("Text",  "text"),
            TackleType("File",  "file"),
            TackleType("Html",  "text file"),
            // TackleType("Image", "Image"),
        ]
    })
}

pub async fn get_tackle_types() -> Json<&'static Vec<TackleType>> {
    Json(get_global_tackle_types())
}
use axum::Json;
use serde::Deserialize;

#[derive(Deserialize, Debug)]
pub struct TestPayload {
    name: String
}


pub async fn path_page(Json(data): Json<TestPayload>) -> &'static str {
    println!("Payload {:?}", data);
    "whateva"
}
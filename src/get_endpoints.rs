use std::sync::OnceLock;

use axum::Json;
use serde::{Deserialize, Serialize};




// should deserialize to something like { "Text" : [ "text" ] }
// where the key is the name of the type 
//  the value is a list of classes used in the html doc
#[derive(Debug, Serialize, Deserialize)]
pub struct TackleType{
    type_name:  String, 
    class_list: String
}

impl TackleType {
    pub fn new(type_name: &str, class_list: &str) -> Self {
        Self {
            type_name: type_name.to_string(), 
            class_list: class_list.to_string()
        }
    }
}


static SECTIONS: OnceLock<Vec<TackleType>> = OnceLock::new();
fn get_global_tackle_types() -> &'static Vec<TackleType> {
    SECTIONS.get_or_init(|| {
        vec![
            TackleType::new("Text", "text"),
            TackleType::new("File", "file"),
            TackleType::new("Html", "text file"),
        ]
    })
}


pub async fn get_tackle_types() -> Json<&'static Vec<TackleType>> {
    Json(get_global_tackle_types())
}
use std::{
    collections::HashMap,
    ops::Not,
    sync::{OnceLock, RwLock},
    time::{SystemTime, UNIX_EPOCH},
};

use axum::{extract::{FromRequest, FromRequestParts, Request}, http::StatusCode};
use serde::Deserialize;
use sha2::Sha256;
use sqlx::{SqlitePool, prelude::FromRow};

use crate::{app_state::AppState, rsa::{Base64String, Hash}};


#[derive(Debug, Clone, Eq, Hash, PartialEq, Deserialize)]
pub struct SessionId(u32);


#[derive(Debug, Clone)]
pub struct Session {
    id: SessionId,
    timestamp: u64,
    username_hash: Base64String<Hash<Sha256>>
}


static SESSION_MAP: OnceLock<RwLock<HashMap<SessionId, Session>>> = OnceLock::new();
fn get_session_map() -> &'static RwLock<HashMap<SessionId, Session>> {
    SESSION_MAP.get_or_init(|| RwLock::new(HashMap::new()))
}

const MAX_SESSION_TIME_SECS: u64 = 2 * 60;

impl Session {
    // i want to avoid using map.retain initially because it requires a &mut and that locks the RwLock
    // removing an item using this is the cold path
    fn remove_old_sessions() {

        let now_secs = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .expect("Failed to get current time")
            .as_secs();
        let oldest_keep_time = now_secs - MAX_SESSION_TIME_SECS;

        let needs_removal = get_session_map()
            .read()
            .expect("Failed to read session map")
            .values()
            .any(|session| session.timestamp < oldest_keep_time);

        if needs_removal {
            get_session_map()
                .write()
                .expect("Failed to access session map as write")
                .retain(|_, last_used| last_used.timestamp >= oldest_keep_time);
        }
    }

    pub fn new(username_hash: Base64String<Hash<Sha256>>) -> Self {
        Self::remove_old_sessions();

        // generate unique id
        let mut id: SessionId;
        loop {
            id = SessionId(rand::random());
            let is_unique = get_session_map()
                .read()
                .expect("Failed to read session map")
                .contains_key(&id)
                .not();

            if is_unique {
                break;
            }
        }

        let now = SystemTime::now();
        let timestamp = now
            .duration_since(UNIX_EPOCH)
            .expect("Not sure how timestamp is before unix epoch")
            .as_secs();



        let new_session = Session {
            id: id.clone(),
            timestamp,
            username_hash
        };

        get_session_map()
            .write()
            .expect("Failed to access SESSION_MAP as write")
            .insert(id, new_session.clone());

        new_session
    }

    pub fn is_valid(&self) -> bool {
        {
            get_session_map().read().unwrap().iter().for_each(|x| println!("SESSION {:?}", x));
        }

        let x = get_session_map()
            .read()
            .unwrap()
            .contains_key(&self.id);

        println!("RETURNING {x}");
        x    
    }

    pub fn get_id_raw(&self) -> u32 {
        self.id.0
    }
}
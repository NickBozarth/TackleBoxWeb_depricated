use std::{
    collections::HashMap,
    ops::Not,
    sync::{OnceLock, RwLock},
    time::{SystemTime, UNIX_EPOCH},
};

use axum::{extract::{FromRequest, FromRequestParts, Request}, http::StatusCode};
use serde::Deserialize;
use sqlx::{SqlitePool, prelude::FromRow};

use crate::app_state::AppState;


#[derive(Debug, Eq, Hash, PartialEq, Deserialize)]
#[serde(transparent)]
pub struct SessionId(pub u32);
#[derive(Debug, Eq, Ord, PartialEq, PartialOrd)]
struct SessionTimestamp(u64);

static SESSION_MAP: OnceLock<RwLock<HashMap<SessionId, SessionTimestamp>>> = OnceLock::new();
fn get_session_map() -> &'static RwLock<HashMap<SessionId, SessionTimestamp>> {
    SESSION_MAP.get_or_init(|| RwLock::new(HashMap::new()))
}

const MAX_SESSION_TIME_SECS: u64 = 2 * 60;

#[derive(Debug, Deserialize)]
pub struct Session {
    #[serde(rename = "session_id")]
    pub id: SessionId
}

impl Session {
    // i want to avoid using map.retain initially because it requires a &mut and that locks the RwLock
    // removing an item using this is the cold path
    fn remove_old_sessions() {
        println!("HERE1");
        let oldest_session_secs = get_session_map()
            .read()
            .expect("Failed to read session map")
            .iter()
            .min_by(|(_, last_used1), (_, last_used2)| last_used1.cmp(last_used2))
            .map(|(_, last_used)| last_used.0);

        // duration since should never fail with the given parameters
        let now_secs = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs();
        if oldest_session_secs.is_some() && now_secs - oldest_session_secs.unwrap() < MAX_SESSION_TIME_SECS { return; }
        let oldest_keep_time = now_secs - MAX_SESSION_TIME_SECS;
        get_session_map()
            .write()
            .expect("Failed to access session map as write")
            .retain(|_, last_used| *last_used >= SessionTimestamp(oldest_keep_time));
    }

    pub fn new() -> Self {
        Self::remove_old_sessions();

        // generate unique id
        let mut id: u32;
        loop {
            id = rand::random();
            let is_unique = get_session_map()
                .read()
                .expect("Failed to read session map")
                .contains_key(&SessionId(id))
                .not();

            println!("ID {} IS UNIQUE {}", id, is_unique);

            if is_unique {
                break;
            }
        }

        let now = SystemTime::now();
        let current_timestamp = now
            .duration_since(UNIX_EPOCH)
            .expect("Not sure how timestamp is before unix epoch")
            .as_secs();

        get_session_map()
            .write()
            .expect("Failed to access SESSION_MAP as write")
            .insert(SessionId(id), SessionTimestamp(current_timestamp));

        println!("SESSION_MAP {:#?}", get_session_map());

        Self {
            id: SessionId(id)
        }
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
}
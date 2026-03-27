use std::{
    collections::HashMap,
    ops::Not,
    sync::{OnceLock, RwLock},
    time::{SystemTime, UNIX_EPOCH},
};

use sqlx::{SqlitePool, prelude::FromRow};

static SESSION_MAP: OnceLock<RwLock<HashMap<u32, u64>>> = OnceLock::new();
fn get_session_map() -> &'static RwLock<HashMap<u32, u64>> {
    SESSION_MAP.get_or_init(|| RwLock::new(HashMap::new()))
}

const MAX_SESSION_TIME_SECS: u64 = 10 * 60;

#[derive(Debug, FromRow)]
pub struct Session(pub u32);

impl Session {
    // i want to avoid using map.retain because it requires a &mut and that locks the RwLock
    // removing an item using this is the cold path
    fn remove_old_sessions() {
        let session_map_read = get_session_map()
            .read()
            .expect("Failed to read session map");
        let oldest_session_secs = session_map_read
            .iter()
            .min_by(|(_, last_used1), (_, last_used2)| last_used1.cmp(last_used2))
            .map(|(_, last_used)| last_used);

        // duration since should never fail with the given parameters
        let now_secs = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_secs();
        if oldest_session_secs.is_some() && now_secs - oldest_session_secs.unwrap() < MAX_SESSION_TIME_SECS { return; }
        let oldest_keep_time = now_secs - MAX_SESSION_TIME_SECS;
        get_session_map()
            .write()
            .expect("Failed to write session map")
            .retain(|_, last_used| *last_used >= oldest_keep_time);
    }

    pub fn new(pool: &SqlitePool) -> Self {
        Self::remove_old_sessions();

        // generate unique id
        let mut id: u32;
        loop {
            id = rand::random();
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
        let current_timestamp = now
            .duration_since(UNIX_EPOCH)
            .expect("Not sure how timestamp is before unix epoch")
            .as_secs();

        get_session_map()
            .write()
            .expect("Failed to access SESSION_MAP as write")
            .insert(id, current_timestamp);

        println!("SESSION_MAP {:#?}", get_session_map());

        Self(id)
    }
}

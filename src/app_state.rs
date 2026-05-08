use sqlx::{Executor, SqlitePool};
use tokio::fs::{self, File};

#[derive(Clone)]
pub struct AppState<'a> {
    pub pool: &'a SqlitePool,
}

impl<'a> AppState<'a> {
    async fn init_db(database_filename: &str) -> &'static mut SqlitePool {
        // create pool
        let pool = Box::leak(Box::new(
            SqlitePool::connect(&("database/".to_owned() + database_filename)).await.expect(""),
        ));

        // ensure file at database_url exists
        File::create(database_filename)
            .await
            .expect("Failed to create database file");

        // initialize tables
        let sql_script = fs::read_to_string("database/create_tables.sql")
            .await
            .expect("Failed to read create_tables.sql");

        pool.execute(sql_script.as_str())
            .await
            .expect("Failed to create tables");

        pool
    }

    pub async fn new() -> AppState<'a> {
        let pool = Self::init_db("database.db").await;
        Self { pool }
    }
}

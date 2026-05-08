CREATE TABLE IF NOT EXISTS logins (
    user_id INTEGER PRIMARY KEY AUTOINCREMENT,
    username_hash VARCHAR NOT NULL,
    login_hash VARCHAR NOT NULL
);

CREATE TABLE IF NOT EXISTS tackles (
    tackle_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NOT NULL REFERENCES logins(user_id),
    iv VARCHAR NOT NULL, -- b64 encoded iv
    created_at INTEGER DEFAULT (unixepoch()),
    tackle_data VARCHAR NOT NULL
);
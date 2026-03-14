#include "server.h"
#include <sqlite3.h>
#include <stdio.h>

void database_ci_dealloc_callback(void* dbptr) {
    sqlite3* db = (sqlite3*)dbptr;
    sqlite3_close(db);
}

EventResponse database_service(Context* context) {
    sqlite3* db;
    int rc;

    rc = sqlite3_open("database/test.db", &db);

    if (rc == SQLITE_OK) {
        ContextItem db_ci = (ContextItem) {
            .name = "database",
            .data = db,
            .dealloc_type = DEALLOC_TYPE_CALLBACK,
            .dealloc_callback = database_ci_dealloc_callback
        };
        ARR_APPEND(*context, db_ci);
        return event_response_ok();
    }
    else {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return event_response_err(500);
    }
}

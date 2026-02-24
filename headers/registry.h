#include "server.h"
#include "page_index.h"
#include "services/key_manager.h"

void register_all_events(HTTPServer* server) {
    register_service(
        server,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        service_key_manager_callback
    );
}

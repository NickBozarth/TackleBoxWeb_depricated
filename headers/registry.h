#include "server.h"
#include "page_index.h"
#include "service_key_manager.h"

void register_all_events(HTTPServer* server) {
    register_event(
        server,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        EVENT_RESPONSE_TYPE_SERVICE, service_key_manager_callback
    );
}

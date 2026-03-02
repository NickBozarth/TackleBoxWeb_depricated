#include "server.h"
#include "page_index.h"
#include "services/key_manager.h"
#include "responders/test_page.h"
#include "processors/default_processor.h"

void register_all_events(HTTPServer* server) {
    register_service(
        server,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        EVENT_DATA_TYPE_MATCH_ALL,   NULL,
        service_key_manager_callback
    );

    register_responder(
        server,
        EVENT_DATA_TYPE_MATCH_STR, "path",
        EVENT_DATA_TYPE_MATCH_STR, "/",
        test_page_callback
    );

    register_processor(
        server,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        default_processor_callback
    );
}

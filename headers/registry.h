#include "RSA.h"
#include "server.h"
#include "page_index.h"



#include "services/key_manager.h"
#include "services/database.h"
#include "services/decryption.h"

#include "responders/test_page.h"
#include "responders/foo.h"

#include "processors/default_processor.h"

void register_all_events(HTTPServer* server) {
    // initialize rsa keyset
    gen_keyset();

    register_service(
        server,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        database_service
    );

    register_service(
        server,
        EVENT_DATA_TYPE_MATCH_STR, "method",
        EVENT_DATA_TYPE_MATCH_STR, "POST",
        decryprion_callback
    ); 

    register_service(
        server,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        service_key_manager_callback
    );

    register_responder(
        server,
        EVENT_DATA_TYPE_MATCH_STR, "path",
        EVENT_DATA_TYPE_MATCH_STR, "/",
        test_page_callback
    );

    register_responder(
        server,
        EVENT_DATA_TYPE_MATCH_STR, "path",
        EVENT_DATA_TYPE_MATCH_STR, "/foo",
        foo_callback
    );

    register_processor(
        server,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        EVENT_DATA_TYPE_MATCH_ALL, NULL,
        default_processor_callback
    );
}

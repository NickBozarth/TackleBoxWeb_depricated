#include "server.h"
#include <stdio.h>

EventResponse test_page_callback(Context *context) {
    Response resp = servefile("index.html", context);
    return event_response_resp(resp);
}

EventResponse test_page_callback1(Context* context) {
    Response resp = servefile("index.html", context);

    return event_response_resp(resp);
}

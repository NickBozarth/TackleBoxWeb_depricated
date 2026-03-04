#include "server.h"
#include "server_utils.h"
#include <stdio.h>

EventResponse test_page_callback(Context *context) {
    
    {

        Cookie* cookie = (Cookie*)malloc(sizeof(Cookie));
        cookie->key = "CLIENT_KEY";
        cookie->value = "CLIENT_KEY_HERE";

        ContextItem cookie_ci = (ContextItem) {
            .name = "Set-Cookie",
            .data = cookie,
            .dealloc_type = DEALLOC_TYPE_FREE
        };
        ARR_APPEND(*context, cookie_ci);
    }

    {
        Cookie* cookie = (Cookie*)malloc(sizeof(Cookie));
        cookie->key = "CLIENT_KEY2";
        cookie->value = "CLIENT_KEY_HERE1223123";

        ContextItem cookie_ci = (ContextItem) {
            .name = "Set-Cookie",
            .data = cookie,
            .dealloc_type = DEALLOC_TYPE_FREE
        };
        ARR_APPEND(*context, cookie_ci);
    }

    {
        Cookie* cookie = (Cookie*)malloc(sizeof(Cookie));
        cookie->key = "CLIENT_KEY3";
        cookie->value = "cookie";

        ContextItem cookie_ci = (ContextItem) {
            .name = "Set-Cookie",
            .data = cookie,
            .dealloc_type = DEALLOC_TYPE_FREE
        };
        ARR_APPEND(*context, cookie_ci);
    }


    Response resp = servefile("index.html", context);
    return event_response_resp(resp);
}


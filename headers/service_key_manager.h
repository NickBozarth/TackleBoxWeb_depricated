#include "server.h"
#include "server_utils.h"
#include <stdio.h>
#include <string.h>

EventResponse service_key_manager_callback(Context* context) {
    Cookies cookies = context_to_cookies(context);
    
    FOR_ITEM_IN_ARR(cookies, cookie) {
        printf("Cookie [%s] -> [%s]\n", cookie->key, cookie->value);
    }

    Cookie* x = get_cookie(cookies, "ClientId");

    if (x != NULL) printf("X = [%s] -> [%s]\n", x->key, x->value);
    else printf("X IS NULL\n");

    return event_response_ok();
    }

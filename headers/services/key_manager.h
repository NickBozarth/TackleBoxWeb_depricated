#include "server.h"
#include "server_utils.h"
#include <stdio.h>
#include <string.h>

EventResponse service_key_manager_callback(Context* context) {
    Cookies cookies = context_to_cookies(context);
    
    Cookie* serverPublicKey = get_cookie(cookies, "server-public-key");
    if (serverPublicKey == NULL) {
        
        set_cookie(context, "server-public-key", "insert-public-key-here", COOKIE_DEALLOC_TYPE_NONE);
    };

    

    return event_response_ok();
}

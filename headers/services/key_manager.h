#include "server.h"
#include "server_utils.h"
#include <stdio.h>
#include <string.h>

EventResponse service_key_manager_callback(Context* context) {
    Cookies cookies = context_to_cookies(context);
    
    Cookie* clientId = get_cookie(cookies, "clientId");
    Cookie* serverPublicKey = get_cookie(cookies, "serverPublicKey");

    if (clientId == NULL || serverPublicKey == NULL) {
    };

    

    return event_response_ok();
}

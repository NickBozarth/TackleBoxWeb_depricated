#include "server.h"
#include "server_utils.h"
#include <stdio.h>
#include <sqlite3.h>

EventResponse test_page_callback(Context *context) {
    Cookies cookies = context_to_cookies(context);
    Cookie* client_id_cookie = get_cookie(cookies, "client-id");
    if(client_id_cookie == NULL)
        set_cookie(context, "client-id", "setting-cookie", COOKIE_DEALLOC_TYPE_NONE);

    print_context_keys(context);


    Response resp = servefile("index.html", context);
    return event_response_resp(resp);
}

#include "server.h"
#include <stdio.h>
#include <string.h>
#include "server_utils.h"


int set_cookies(Context* context, Response* resp) {
    FOR_ITEM_IN_ARR(*context, ci) {
        if (strcasecmp(ci->name, "Set-Cookie") != 0) continue;
        Cookie* cookie = (Cookie*)ci->data;
        

        // format cookie as a header
        //                                   account for '=' and '\0'   vvv
        size_t chs_size = (strlen(cookie->key) + strlen(cookie->value) + 2) * sizeof(char);
        char* cookie_value_str = (char*)malloc(chs_size);
        snprintf(cookie_value_str, chs_size, "%s=%s", cookie->key, cookie->value);

        HTTPHeader cookie_header = (HTTPHeader) {
            .name  = "Set-Cookie",
            .value = cookie_value_str,
            .dealloc_type = DEALLOC_TYPE_CALLBACK,
            .dealloc_callback = dealloc_httpheader_value
        };

        ARR_APPEND(resp->headers, cookie_header);
    }

    return 0;
}



int set_headers(Context* context, Response* resp) {
    FOR_ITEM_IN_ARR(*context, ci) {
        // filter for all context items that are named Set-Header
        if (strcasecmp(ci->name, "Set-Header") != 0) continue;

        ARR_APPEND(resp->headers, *((HTTPHeader*)ci->data) );
    }

    return 0;
}



Response default_processor_callback(Context* context, Response resp) {
    set_cookies(context, &resp);
    set_headers(context, &resp);
    return resp;
}

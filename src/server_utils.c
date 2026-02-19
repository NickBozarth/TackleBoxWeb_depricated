#include "server_utils.h"
#include "server.h"
#include <string.h>

#include <stdio.h>


Cookie cookie_from_str(char** cookie_str_ptr) {
    char* key_str = *cookie_str_ptr;
    char* value_str = strchr(key_str, '=');
    if (value_str != NULL) {
        *value_str = '\0';
        value_str++;
        *cookie_str_ptr = value_str;
    }


    return (Cookie) {
        .key   = key_str,
        .value = value_str,
    };
}

Cookies context_to_cookies(Context *context) {
    Cookies cookies = {};
    
    // extract headers from context
    if (context->item_count < 1) return cookies;
    HTTPResponse* resp = (HTTPResponse*)context->items[0].data;
    // extract cookie header from headers
    HTTPHeader* cookie_header = NULL;
    FOR_ITEM_IN_ARR(resp->headers, header) {
        if (strcmp(header->name, "Cookie") == 0) {
            cookie_header = header;
            break;
        } 
    }
    if (cookie_header == NULL) return cookies;

    // break apart cookies by '; ' delimiter
    char* cookies_str = cookie_header->value;
    ARR_APPEND(cookies, cookie_from_str(&cookies_str));
    printf("BEFORE %s\n", cookies_str);
    while ((cookies_str = strchr(cookies_str, ';')) != NULL) {
        // before
        // "cookie1; cookie2; ..."
        // ^ cookies_str
        *cookies_str = '\0';
        cookies_str += 2;
        // **after**
        // "cookie1\0 cookie2; ..."
        //            ^ cookies_str
        printf("AFTER %s\n", cookies_str);
        ARR_APPEND(cookies, cookie_from_str(&cookies_str));
    }
    
    return cookies;
}


Cookie* get_cookie(Cookies cookies, char* key) {
    return ARR_GET(cookies, cookie, strcmp(cookie->key, key) == 0);
}

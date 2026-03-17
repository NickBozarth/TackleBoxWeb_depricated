#include "server.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "server_utils.h"


void str_replace_newline(char** str) {
    // check for position of newline
    char* newline_ptr = strchr(*str, '\n');
    if (newline_ptr == NULL) return;

    // figure out how long the string pre-newline is
    ptrdiff_t newline_offset = newline_ptr - *str;

    // make buffers for pre-newline and new strings
    size_t new_str_len = strlen(*str) + 3;
    char* new_str = malloc(new_str_len * sizeof(char)); // make new buffer with space for %0A
    
    // [everything pre-newline]%0A[%everything post-newline]
    snprintf(
        new_str, 
        new_str_len, 
        "%.*s%%0A%s", 

        (int)newline_offset, *str, 
        *str + newline_offset + 1
    );
    
    free(*str);
    *str = new_str;
    str_replace_newline(str);
}


int set_cookies(Context* context, Response* resp) {
    FOR_ITEM_IN_ARR(*context, ci) {
        if (strcasecmp(ci->name, "Set-Cookie") != 0) continue;
        Cookie* cookie = (Cookie*)ci->data;

        if (
            cookie->dealloc_type == COOKIE_DEALLOC_TYPE_KEY ||
            cookie->dealloc_type == COOKIE_DEALLOC_TYPE_BOTH
        ) {
            str_replace_newline(&cookie->key);
        }

        if (
            cookie->dealloc_type == COOKIE_DEALLOC_TYPE_VALUE ||
            cookie->dealloc_type == COOKIE_DEALLOC_TYPE_BOTH
        ) {
            str_replace_newline(&cookie->value);
        }

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

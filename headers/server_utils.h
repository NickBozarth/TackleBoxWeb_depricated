#pragma once

#include "server.h"


typedef enum {
    COOKIE_DEALLOC_TYPE_NONE = 0,
    COOKIE_DEALLOC_TYPE_KEY,
    COOKIE_DEALLOC_TYPE_VALUE,
    COOKIE_DEALLOC_TYPE_BOTH,
} CookieDeallocType_e;

// TODO add other options for cookies, ex: Secure, HTTPOnly, SameSite
typedef struct {
    char* key;
    char* value;

    CookieDeallocType_e dealloc_type;
} Cookie;

typedef DYNARR(Cookie) Cookies;

Cookies context_to_cookies(Context* context);
Cookie* get_cookie(Cookies cookies, char* key);
void set_cookie(Context* context, char* key, char* value, CookieDeallocType_e);

void cookie_ci_dealloc_callback(void*);
void dealloc_cookie(Cookie*);

void print_context_keys(Context*);

#pragma once

#include "server.h"


// TODO add other options for cookies, ex: Secure, HTTPOnly, SameSite
typedef struct {
    char* key;
    char* value;
} Cookie;

typedef DYNARR(Cookie) Cookies;

Cookies context_to_cookies(Context* context);
Cookie* get_cookie(Cookies cookies, char* key);

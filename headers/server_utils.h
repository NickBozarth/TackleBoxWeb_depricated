#include "server.h"

typedef struct {
    char* key;
    char* value;
} Cookie;

typedef DYNARR(Cookie) Cookies;

Cookies context_to_cookies(Context* context);
Cookie* get_cookie(Cookies cookies, char* key);

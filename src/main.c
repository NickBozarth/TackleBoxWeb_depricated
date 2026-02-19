#include <stdio.h>
#include "registry.h"

#include <stdbool.h>
#include <string.h>

bool match_path(char* path) {
    return strlen(path) > 3;
}

char* callback(Context* context) {
    return "Hello";
}


int main() {
    HTTPServer server = {};
    register_all_events(&server);
    serve(&server);
}

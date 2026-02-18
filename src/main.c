#include <stdio.h>
#include "server.h"

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
    
    register_event(
            &server, 
            EVENT_DATA_TYPE_MATCH_STR, "path", 
            EVENT_DATA_TYPE_CALLBACK,  match_path,
            EVENT_RESPONSE_TYPE_RESPONDER, callback
    );


    serve(&server);
}

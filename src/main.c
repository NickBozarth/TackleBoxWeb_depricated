#include <stdio.h>
#include "server.h"


HTTPResponse callback(HTTPHeader header) {
    printf("reached callback");
    return (HTTPResponse) {};
}


int main() {
    HTTPServer server = {};
    serve(&server);
}

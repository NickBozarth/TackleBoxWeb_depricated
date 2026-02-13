#include <stdio.h>
#include "server.h"

int main() {
    HTTPServer server = {};
    serve(&server);
}

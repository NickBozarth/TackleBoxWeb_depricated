#include "registry.h"

int main() {
    HTTPServer server = {};
    register_all_events(&server);
    serve(&server);
}

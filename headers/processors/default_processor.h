#include "server.h"
#include <stdio.h>

Response default_processor_callback(Context* context, Response* httpresp) {
    return *httpresp;
}

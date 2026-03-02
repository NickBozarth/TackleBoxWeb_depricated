#include "server.h"
#include <stdio.h>

Response default_processor_callback(Context* context, Response* httpresp) {
    if (httpresp != NULL) {
        printf("~~~~~~~~~~~~~~~~\n");
        printresp(&httpresp->data.http);
        printf("~~~~~~~~~~~~~~~~\n");
    }

    Response resp = {};
    resp.type = RESPONSE_TYPE_HTTP;
    resp.data.http = httpresp->data.http;

    return resp;
}

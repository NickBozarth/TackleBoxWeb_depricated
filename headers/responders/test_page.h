#include "server.h"
#include <stdio.h>

EventResponse test_page_callback(Context *context) {
    Response resp = servefile("index.html", context);
    return event_response_resp(resp);
}

EventResponse test_page_callback1(const Context* context) {
    HTTPResponse httpresp = {};
    httpresp.method = "HTTP/1.1";
    httpresp.path = "200";
    httpresp.protocol = "OK";
    HTTPHeader h1 = (HTTPHeader) { "Content-Type", "text/plain" };
    HTTPHeader h2 = (HTTPHeader) { "Content-Length", "12" };
    ARR_APPEND(httpresp.headers, h1);
    ARR_APPEND(httpresp.headers, h2);
    httpresp.body = "Hello World!";


    printf("test page constructed response\n");

    Response resp = {};
    resp.type = RESPONSE_TYPE_HTTP;
    resp.data.http = httpresp;

    return event_response_resp(resp);
}

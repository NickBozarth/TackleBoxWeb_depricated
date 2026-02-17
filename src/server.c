#include "server.h"
#include <stdio.h>
#include <string.h>


void printresp(HTTPResponse *resp) {
    printf("METHOD: [%s]\n", resp->method);
    printf("PATH: [%s]\n", resp->path);
    printf("PROTOCOL: [%s]\n", resp->protocol);

    FOR_I_IN_ARR(resp->headers) {
        HTTPHeader cur_header = resp->headers.items[i];
        printf("HEADER [%s] -> VALUE [%s]\n", cur_header.header, cur_header.value);
    }

    printf("BODY LENGTH [%lu]\n", resp->body_len);
    printf("BODY {\n\t");
    for (int i = 0; i < resp->body_len; i++) {
        printf("%c", resp->body[i]);
        if (resp->body[i] == '\n') printf("\t");
    }
    printf("\n}\n");
}




HTTPResponse bufftoresp(char *buffer, size_t buffer_len) {
    HTTPResponse resp = {};
    char* method = buffer;
    char* path = strchr(buffer, ' ');
    *path = '\0';
    path++;
    char* protocol = strchr(path, ' ');
    *protocol = '\0';
    protocol++;
    char* headers_str = strstr(protocol, "\r\n");
    *headers_str = '\0';
    headers_str += 2;

    char* body = strstr(headers_str, "\r\n\r\n");
    size_t body_len = 0;
    if (body != NULL) {
        *body = '\0';
        body += 4;
        body_len = buffer_len - (unsigned long)body + (unsigned long)buffer;
    }



    char* state;
    char* token = strtok_r(headers_str, "\r\n", &state);
    while (token != NULL) {
        char* header = token;
        char* value = strchr(header, ':');
        if (value != NULL) {
            *value = '\0';
            value++;
            // get rid of whitespace
            while (*value == ' ') value++;

            HTTPHeader httpHeader = { header, value };
            ARR_APPEND(resp.headers, httpHeader);
        }

        token = strtok_r(NULL, "\r\n", &state);
    }


    resp.method = method;
    resp.path = path;
    resp.protocol = protocol;

    resp.body = body;
    resp.body_len = body_len;

    printresp(&resp);

    return resp;
}



int register_event(
        HTTPServer* server,
        HTTPEventDataType_e header_type,       void* header_data,
        HTTPEventDataType_e value_type,        void* value_data,
        HTTPEventResponseType_e response_type, void* callback
) {
   HTTPEvent event = (HTTPEvent) {
        .id = server->responders.item_count + server->services.item_count,
        .header_data = {
            .type = header_type,
            .data = header_data,
        },

        .value_data = {
            .type = value_type,
            .data = value_data,
        },

        .resp = {
            .type = response_type,
            .callback = callback
        }
   };


   switch (response_type) {
        case EVENT_RESPONSE_TYPE_SERVICE:
            ARR_APPEND(server->services, event);
            return event.id;
        break;
        case EVENT_RESPONSE_TYPE_RESPONDER:
            ARR_APPEND(server->responders,event);
            return event.id;
        break;
        default:
            return -1;
        break;
   } 
}


char* eval_events(HTTPServer *server, HTTPResponse *resp) {
    exit(1); // TODO
}

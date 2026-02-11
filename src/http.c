#include "http.h"
#include <stdio.h>
#include <string.h>


void printresp(HTTPResponse* resp) {
    printf("METHOD: %s\n", resp->method);
    printf("PATH: %s\n", resp->path);
    printf("PROTOCOL: %s\n\n", resp->protocol);

    printf("HEADER COUNT %lli\n", resp->headers.item_count);
    FOR_I_IN_ARR(resp->headers) {
        printf("Header: [%s] -> Value: [%s]\n", resp->headers.items[i].key, resp->headers.items[i].value);
    }

    // printf("\nBODY: {\n%s\n}\n", resp->body);
    printf("\nBODY: {\n\t");
    for (int i = 0; i < resp->body_len; i++) {
        printf("%c", resp->body[i]);
        if (resp->body[i] == '\n') printf("\t");
    }
    printf("\n}\n");
}


HTTPResponse bufftoresp(char* buffer, size_t buffer_len) {
    buffer[buffer_len-1] = '\0';
    HTTPResponse resp = {};
    char* line;
    char* saveptr;
    // "headers are here\r\n\r\nbody is here"
    // "headers are here\0\n\r\nbody is here"
    //                          ^ body_start is here
    char* body_start = strstr(buffer, "\r\n\r\n");
    if (body_start) {
        *body_start = '\0';
        body_start += 4;
    }
    
    // method, path, and protocol should be along the begining delimited by spaces
    line = strtok_r(buffer, " ", &saveptr);
    if (!line) return resp;
    resp.method = line;
    line = strtok_r(NULL, " ", &saveptr);
    if (!line) return resp;
    resp.path = line;
    line = strtok_r(NULL, "\r\n", &saveptr);
    if (!line) return resp;
    resp.protocol = line;

    // headers should be delimited by "\r\n"
    while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
        // header:value pairs should be delimited by ':'
        // values might start with whitespace
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';

            char* header = line;
            char* value  = colon + 1;

            // get rid of whitespace
            while (*value == ' ') value++;

            // HTTPHeader* new_buffer = (HTTPHeader*)realloc(resp.headers, sizeof(HTTPHeader) * (resp.header_count + 1));
            // if (new_buffer == NULL) continue;
            // resp.headers = new_buffer;
            // resp.headers[resp.header_count] = (HTTPHeader){ header, value };
            // resp.header_count++;
            HTTPHeader new_item = (HTTPHeader){ header, value };
            ARR_APPEND(resp.headers, new_item);

            HTTPHeader h = ARR_ITEM_AT(resp.headers, 1);
        }
    }


    resp.body = body_start;
    if (body_start) resp.body_len = buffer_len - (body_start - buffer);
    else resp.body = 0;

    printresp(&resp);

    return resp;
}


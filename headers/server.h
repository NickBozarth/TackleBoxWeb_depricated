#pragma once

#include <stddef.h>
#include "dynamic_arr.h"
#include "event.h"

#define PORT 8080
#define PORT_STR "8080"
#define SERVE_DIRECTORY ".\\serve\\"
#define MAX_FILENAME_LEN 256
#define BUFFER_SIZE 1024

#ifdef _WIN32
    // TODO define socket for windows
#else
    typedef int SOCKET;
#endif



typedef struct {
    char* header;
    char* value;
} HTTPHeader;


typedef struct {
    char* method;
    char* path;
    char* protocol;

    DYNARR(HTTPHeader) headers;

    char* body;
    size_t body_len;
} HTTPResponse;



DYNARR(HTTPEvent);
typedef struct {
    struct HTTPEvent__Dynarr services;
    struct HTTPEvent__Dynarr responders;
} HTTPServer;


void printresp(HTTPResponse* resp);
HTTPResponse bufftoresp(char* buffer, size_t buffer_len);

int register_event(
        HTTPServer* server,
        HTTPEventDataType_e header_type,       void* header_data,
        HTTPEventDataType_e value_type,        void* value_data,
        HTTPEventResponseType_e response_type, void* callback
);
char* eval_events(HTTPServer* server, HTTPResponse* resp);

void send_client();
int servefile(SOCKET clientSocket, char* filePath);
void serve(HTTPServer* server);

// void printresp(HTTPResponse* resp);

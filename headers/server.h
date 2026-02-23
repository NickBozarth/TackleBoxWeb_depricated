#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "dynamic_arr.h"
// #include "event.h"

#define PORT 8080
#define PORT_STR "8080"
#define SERVE_DIRECTORY "./serve/"
// #define SERVE_DIRECTORY ".\\serve\\"
#define MAX_FILENAME_LEN 256
#define BUFFER_SIZE 1024

#ifdef _WIN32
    // TODO define socket for windows
#else
    typedef int SOCKET;
#endif



typedef struct {
    char* name;
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


typedef struct {
    enum {
        EVENT_RESPONSE_OK,      // event executed ok and will continue execution
        EVENT_RESPONSE_ERR,     // event failed execution and server will respond with err
        EVENT_RESPONSE_RESPOND, // event created response and server will respond to client
    } type;

    union {
        void* ok;
        int err;
        HTTPResponse resp;
    } data;
} EventResponse;


typedef struct {
    char* name;
    void* data;
    enum {
        NO_FREE = 0,
        C_FREE,
        CALLBACK_DEALLOC
    } dealloc_type;
    void (*dealloc_function)(void*);
} ContextItem;

typedef DYNARR(ContextItem) Context;


typedef enum {                      // typeof data
    EVENT_DATA_TYPE_MATCH_STR,      // char*
    EVENT_DATA_TYPE_CALLBACK,       // EventResponse (*callback)(HTTPResponse*)
    EVENT_DATA_TYPE_MATCH_ALL,      // void
} HTTPEventDataType_e;

typedef struct {
    HTTPEventDataType_e type;
    void* data;
} HTTPEventData;


typedef enum {
    EVENT_RESPONSE_TYPE_SERVICE,    // SERVICEs mainly change the context
                                    // typeof callback -> EventResponse(*)(Context*)
                                    // returning OK continues processing other services
                                    // returning ERR or RESP halts other service execution and
                                    //  skips all responders 
    EVENT_RESPONSE_TYPE_RESPONDER,  // RESPONDER will provide a response to send to the client
                                    // typeof callback -> EventResponse(*)(Context*)
                                    // returning OK halts responder execution and returns ERR(500)
                                    // returning ERR or RESP halts other responder execution
                                    // IF no RESPONDERs return OK, return ERR(404)
    EVENT_RESPONSE_TYPE_PROCESSER,  // PROCESSORs assist assembly of the response before sending
                                    // typeof callback -> EventResponse(*)(Context*,HTTPResponse*)
                                    // returning OK continues processing other processors
                                    // returning ERR assembles a fallback error response and 
                                    //  sends it to client
                                    // returning RESP assembles response and sends it to client
                                    // IF no PROCESSORs return RESP, fallback response will be
                                    //  assembled and sent to client
} HTTPEventResponseType_e;

typedef struct {
    HTTPEventResponseType_e type;
    EventResponse (*callback)(Context*);
} HTTPEventResponse;


typedef struct {
    int id;

    HTTPEventData header_data;
    HTTPEventData value_data;

    HTTPEventResponse resp;
} HTTPEvent; 



DYNARR(HTTPEvent);
typedef struct {
    struct HTTPEvent__Dynarr services;
    struct HTTPEvent__Dynarr responders;
    struct HTTPEvent__Dynarr processors;
} HTTPServer;







EventResponse event_response_ok();
EventResponse event_response_err(int err);
EventResponse event_response_resp(HTTPResponse resp);

void printresp(HTTPResponse* resp);
HTTPResponse bufftoresp(char* buffer, size_t buffer_len);

int register_event(
        HTTPServer* server,
        HTTPEventDataType_e header_type,       void* header_data,
        HTTPEventDataType_e value_type,        void* value_data,
        HTTPEventResponseType_e response_type, EventResponse (*callback)(Context*)
);
EventResponse eval_events(HTTPServer* server, HTTPResponse* resp, SOCKET* clientSocket);

void send_client(SOCKET clientSocket, char* buf, size_t buf_size);
int servefile(SOCKET clientSocket, const char* filePath);
void serve(HTTPServer* server);

// void printresp(HTTPResponse* resp);

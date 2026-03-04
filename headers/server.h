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

#define ERR_RETURN_RESP_TEMPLATE    \
    "HTTP/1.1 200 OK\n"             \
    "Content-Type: text/plain\n"    \
    "Content-Length: 12\n"          \
    "\n"

#ifdef _WIN32
    // TODO define socket for windows
#else
    typedef int SOCKET;
#endif



typedef enum {
    DEALLOC_TYPE_NONE = 0,
    DEALLOC_TYPE_FREE,
    DEALLOC_TYPE_CALLBACK,
} DeallocType_e;

typedef void (*DeallocFunction)(void*);


typedef struct {
    char* name;
    void* data;
    DeallocType_e dealloc_type;
    DeallocFunction dealloc_callback;
} ContextItem;

typedef DYNARR(ContextItem) Context;


typedef struct {
    char* name;
    char* value;
    DeallocType_e dealloc_type;
    DeallocFunction dealloc_callback;
} HTTPHeader;

DYNARR(HTTPHeader);

typedef struct {
    char* method;
    char* path;
    char* protocol;

    struct HTTPHeader__Dynarr headers;

    char* body;
    size_t body_len;
} HTTPResponse;


typedef struct {
    char* str;
    size_t size;
    DeallocType_e dealloc_type;
    DeallocFunction dealloc_callback;
} StringObject;

typedef int (*ResponseCallback)(struct HTTPHeader__Dynarr, Context*);

typedef struct {
    enum {
        RESPONSE_TYPE_NONE = 0,
        RESPONSE_TYPE_ERR,
        RESPONSE_TYPE_STR,
        RESPONSE_TYPE_CALLBACK,
    } type;

    struct HTTPHeader__Dynarr headers;

    union {
        void* none;
        int err;
        StringObject str;
        ResponseCallback callback;
    } body;
} Response;



typedef struct {
    enum {
        EVENT_RESPONSE_OK,      // event executed ok and will continue execution
        EVENT_RESPONSE_ERR,     // event failed execution and server will respond with err
        EVENT_RESPONSE_RESPOND, // event created response and server will respond to client
    } type;

    Response resp;
} EventResponse;







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
                                    // typeof callback -> EventResponse(*)(const Context*)
                                    // returning OK halts responder execution and returns ERR(500)
                                    // returning ERR or RESP halts other responder execution
                                    // IF no RESPONDERs return OK, return ERR(404)
    EVENT_RESPONSE_TYPE_PROCESSER,  // PROCESSORs assist assembly of the response before sending
                                    // typeof callback -> StringResp(*)(Context*,HTTPResponse*)
                                    // returning StringResp.resp == NULL will return a fallback
                                    //  error page to client
                                    // returning StringResp.resp != NULL will return
                                    //  StringResp.resp to client then dealloc the resp object
                                    //  according to StringResp.dealloc_type
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
EventResponse event_response_resp(Response resp);

void printresp(HTTPResponse* resp);
HTTPResponse bufftoresp(char* buffer, size_t buffer_len);

typedef EventResponse (*ServiceCallback)(Context*);
typedef EventResponse (*ResponderCallback)(Context*);
typedef Response (*ProcessorCallback)(Context*, Response);

int register_service(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    ServiceCallback callback
);
int register_responder(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    ResponderCallback callback
);
int register_processor(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    ProcessorCallback callback
);

Response servefile(const char* filePath, Context* context);

Response eval_events(HTTPServer* server, HTTPResponse* resp, SOCKET* clientSocket, Context*);

void send_resp(SOCKET clientSocket, Response resp, Context* context);
void send_client(SOCKET clientSocket, char* buf, size_t buf_size);
void serve(HTTPServer* server);



void dealloc_httpheader_name(void*);
void dealloc_httpheader_value(void*);
void dealloc_httpheader_both(void*);

void dealloc_stringobject(void*);
StringObject stringwithsize_h(size_t size);


// void printresp(HTTPResponse* resp);

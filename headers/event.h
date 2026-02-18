#include "dynamic_arr.h"

typedef struct {
    char* name;
    void* data;
} ContextItem;


typedef struct {
    DYNARR(ContextItem) items;
} Context;



typedef enum {                      // typeof data
    EVENT_DATA_TYPE_MATCH_STR,      // char*
    EVENT_DATA_TYPE_CALLBACK,       // header_data -> char* (*callback)(HTTPResponse*)
                                    // value_data  -> bool  (*callback)(char*)
    EVENT_DATA_TYPE_MATCH_ALL,      // void
} HTTPEventDataType_e;

typedef struct {
    HTTPEventDataType_e type;
    void* data;
} HTTPEventData;


typedef enum {                      // typeof callback
    EVENT_RESPONSE_TYPE_SERVICE,    //   int (*f) (Context*)
    EVENT_RESPONSE_TYPE_RESPONDER,  // char* (*f) (Context*)
} HTTPEventResponseType_e;

typedef struct {
    HTTPEventResponseType_e type;
    void* callback;
} HTTPEventResponse;


typedef struct {
    int id;

    HTTPEventData header_data;
    HTTPEventData value_data;

    HTTPEventResponse resp;
} HTTPEvent; 


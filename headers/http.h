#include <stddef.h>
#include "dynamic_arr.h"


typedef struct {
    char* key;
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

void printresp(HTTPResponse* resp);
HTTPResponse bufftoresp(char* buffer, size_t buffer_len);
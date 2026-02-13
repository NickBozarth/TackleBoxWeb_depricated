#include <stddef.h>
#include "dynamic_arr.h"


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


typedef enum {                   // typeof data in listener
    HTTPLISTENERTYPE_MATCH_NAME, // char* 
} HTTPListenerType_e;

typedef struct {
    HTTPListenerType_e type;
    char* header;
    void* data;
} HTTPListener;


typedef struct {
    
} HTTPServer;


void serve(HTTPServer* server);

void send_client();

// void printresp(HTTPResponse* resp);
//HTTPResponse bufftoresp(char* buffer, size_t buffer_len);

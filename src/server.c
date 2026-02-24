#include "server.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



EventResponse event_response_ok() {
    return (EventResponse) {
        .type = EVENT_RESPONSE_OK,
        .data.ok = NULL
    };
}

EventResponse event_response_err(int err) {
    return (EventResponse) {
        .type = EVENT_RESPONSE_ERR,
        .data.err = err,
    };
}

EventResponse event_response_resp(HTTPResponse resp) {
    return (EventResponse) {
        .type = EVENT_RESPONSE_RESPOND,
        .data.resp = resp
    };
}


void printresp(HTTPResponse *resp) {
    printf("METHOD: [%s]\n", resp->method);
    printf("PATH: [%s]\n", resp->path);
    printf("PROTOCOL: [%s]\n", resp->protocol);

    FOR_I_IN_ARR(resp->headers) {
        HTTPHeader cur_header = resp->headers.items[i];
        printf("HEADER [%s] -> VALUE [%s]\n", cur_header.name, cur_header.value);
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
        case EVENT_RESPONSE_TYPE_PROCESSER:
            ARR_APPEND(server->processors, event);
            return event.id;
        break;
   } 

   return -1;
}

int register_service(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    EventResponse (*callback)(Context*)
) {
    return register_event(
        server, 
        header_type, header_data, 
        value_type, value_data, 
        EVENT_RESPONSE_TYPE_SERVICE, (void*)callback
    );
}

int register_responder(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    EventResponse (*callback)(const Context*)
) {
    return register_event(
        server, 
        header_type, header_data, 
        value_type, value_data, 
        EVENT_RESPONSE_TYPE_RESPONDER, (void*)callback
    );
}

int register_processor(
    HTTPServer* server,
    HTTPEventDataType_e header_type,       void* header_data,
    HTTPEventDataType_e value_type,        void* value_data,
    EventResponse (*callback)(Context*, HTTPResponse*)
) {
    return register_event(
        server, 
        header_type, header_data, 
        value_type, value_data, 
        EVENT_RESPONSE_TYPE_PROCESSER, (void*)callback
    );
}





// returns the value of the matched header
// returns NULL if no header is found to match
char* event_header_matches(HTTPEventData* header_data, HTTPResponse* resp) {
    switch (header_data->type) {
        case EVENT_DATA_TYPE_MATCH_ALL:
            return (char*)1;
        break;
        case EVENT_DATA_TYPE_MATCH_STR:
            printf("header data -> [%s]\n", (char*)header_data->data);
            printf("resp path -> [%s]\n", resp->path);

            // special cases for non headers
            if (strcmp((char*)header_data->data, "method")   == 0) return resp->method;
            if (strcmp((char*)header_data->data, "path")     == 0) return resp->path;
            if (strcmp((char*)header_data->data, "protocol") == 0) return resp->protocol;
            
            // check all headers for a match
            char* header_name = (char*)header_data->data;
            FOR_ITEM_IN_ARR(resp->headers, header) {
                if (strcmp(header_name, header->name) == 0) return header->value;
            }
            return NULL;
        break;
        case EVENT_DATA_TYPE_CALLBACK:
            return ((char* (*)(HTTPResponse*))header_data->data)(resp);
        break;
    }

    return NULL;
}

bool event_value_matches(HTTPEventData* value_data, char* value) {
    switch (value_data->type) {
        case EVENT_DATA_TYPE_MATCH_ALL:
            return true;
        break;
        case EVENT_DATA_TYPE_MATCH_STR:
            return strcmp((char*)value_data->data, value) == 0;
        break;
        case EVENT_DATA_TYPE_CALLBACK:
            return ((bool (*)(char*))value_data->data)(value);
        break;
    }

    return false;
}

bool event_matches(HTTPEvent* event, HTTPResponse* resp) {
    char* ret = event_header_matches(&event->header_data, resp);
    if (ret == NULL)     return false;
    if (ret == (char*)1) return true;

    return event_value_matches(&event->value_data, ret);
}


EventResponse eval_events(HTTPServer *server, HTTPResponse *client_resp, SOCKET* clientSocket) {
    Context context = {};
    ContextItem resp_ci = (ContextItem) {
        .name = "response",
        .data = client_resp,
        .dealloc_type = NO_FREE,
    };
    ContextItem socket_ci = (ContextItem) {
        .name = "clientSocket",
        .data = clientSocket,
        .dealloc_type = NO_FREE,
    };
    ARR_APPEND(context, resp_ci);
    ARR_APPEND(context, socket_ci);


    // EVENT_RESPONSE_OK will serve as an unset because there is no case that uses OK and returns
    //  a resonse to the client as returning an OK response to clients will result in a 500 error
    EventResponse server_resp = { .type = EVENT_RESPONSE_OK };
    FOR_ITEM_IN_ARR(server->services, service) {
        // filter for any matching event
        if (!event_matches(service, client_resp)) continue;

        // form and call the callback attached to the event
        EventResponse (*event_callback)(Context*);
        event_callback = (EventResponse(*)(Context*))service->resp.callback;
        EventResponse eres = event_callback(&context);

        // process the event
        switch (eres.type) {
            case EVENT_RESPONSE_OK:
                continue;
            break;

            case EVENT_RESPONSE_ERR:
            case EVENT_RESPONSE_RESPOND:
                server_resp = eres;
                goto PROCESS_RESP;
            break;

            default:
                server_resp = event_response_err(500);
                printf("Service event [id : %i] returned with invalid type.\n", service->id);
                goto PROCESS_RESP;
            break;
        }
    }


    FOR_ITEM_IN_ARR(server->responders, responder) {
        if (!event_matches(responder, client_resp)) continue;

        EventResponse (*event_callback)(const Context*);
        event_callback = (EventResponse(*)(const Context*))responder->resp.callback;
        EventResponse eres = event_callback(&context);
        switch (eres.type) {
            case EVENT_RESPONSE_OK:
                server_resp = event_response_err(500);
                goto  PROCESS_RESP;
            break;

            case EVENT_RESPONSE_ERR:
            case EVENT_RESPONSE_RESPOND:
                server_resp = eres;
                goto PROCESS_RESP;
            break;

            default:
                server_resp = event_response_err(500);
                printf("Responder event [id : %i] returned with invalid type.\n", responder->id);
            break;
        }
    }


PROCESS_RESP:

    switch (server_resp.type) {
        case EVENT_RESPONSE_RESPOND: // process events
        break;

        case EVENT_RESPONSE_OK: // treat OK as a page not found error
            server_resp = event_response_err(404);
        case EVENT_RESPONSE_ERR:
            goto CLEANUP;
        break;

        default:
            server_resp = event_response_err(500);
            goto CLEANUP;
        break;
    }


    // TODO unsure about implementing this here, in switch statement, or elsewhere
    FOR_ITEM_IN_ARR(server->processors, processor) {
        if (!event_matches(processor, client_resp)) continue;
        
        EventResponse (*event_callback)(Context*, HTTPResponse*);
        event_callback = (EventResponse(*)(Context*, HTTPResponse*))processor->resp.callback;
        EventResponse eres = event_callback(&context, &server_resp.data.resp);
        switch (eres.type) {
            case EVENT_RESPONSE_OK:
                continue;
            break;

            case EVENT_RESPONSE_ERR:
            case EVENT_RESPONSE_RESPOND:
                server_resp = eres;
                goto CLEANUP;
            break;

            default:
                printf("Processor event [id : %i] returned with invalid type.\n", processor->id);
                server_resp = event_response_err(500);
            break;
        }
    }



CLEANUP:

    // handle deallocation for context items
    FOR_ITEM_IN_ARR(context, context_item) {
        switch (context_item->dealloc_type) {
            case NO_FREE:
            break;
            case C_FREE:
                free(context_item->data);
            break;
            case CALLBACK_DEALLOC:
                context_item->dealloc_function(context_item->data);
            break;
            default:
                printf(
                        "Context item [%s] does not contain a supported dealloc type.\n", 
                        context_item->name
                );
            break;
        }
    }

    return event_response_err(404);
}














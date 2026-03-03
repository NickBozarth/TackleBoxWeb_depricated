#include <asm-generic/socket.h>
#include <stdbool.h>
#ifndef _WIN32

#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>




void send_client(SOCKET clientSocket, char *buf, size_t buf_size) {
    printf("sent client %lu bytes\n", buf_size);
    write(clientSocket, buf, buf_size);
}



int servefile_internal(Context* context) {
    ContextItem* socket_ci = ARR_GET(*context, ci, strcmp(ci->name, "clientSocket") == 0);
    if (socket_ci == NULL) return 1;
    SOCKET client_socket = *((SOCKET*)socket_ci->data);
    ContextItem* filepath_ci = ARR_GET(*context, ci, strcmp(ci->name, "serve_file") == 0);
    if (filepath_ci == NULL) return 2;
    char* filepath = (char*)filepath_ci->data;

    FILE* fp;
    char full_file_path[sizeof(SERVE_DIRECTORY) + MAX_FILENAME_LEN];
    snprintf(full_file_path, sizeof(full_file_path), "%s%s", SERVE_DIRECTORY, filepath);

    fp = fopen(full_file_path, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file [%s]\n", full_file_path);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[512];
    int headerLen = snprintf(
        header,
        sizeof(header),
        
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %li\r\n"
        "\r\n",

        fileSize
    );

    send_client(client_socket, header, headerLen);

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send_client(client_socket, buffer, bytesRead);
    }

    fclose(fp);
    return 0;

}



Response servefile(const char* filepath, Context* context) {
    ContextItem filepath_ci = (ContextItem) {
        .name = "serve_file",
        .data = (void*)filepath,
        .dealloc_type = DEALLOC_TYPE_NONE
    };
    ARR_APPEND(*context, filepath_ci);

    return (Response) {
        .type = RESPONSE_TYPE_CALLBACK,
        .data.callback = servefile_internal
    };
}


void free_context(Context* context) {
    // handle deallocation for context items
    FOR_ITEM_IN_ARR(*context, context_item) {
        switch (context_item->dealloc_type) {
            case DEALLOC_TYPE_NONE:
            break;
            case DEALLOC_TYPE_FREE:
                free(context_item->data);
            break;
            case DEALLOC_TYPE_CALLBACK:
                context_item->dealloc_callback(context_item->data);
            break;
            default:
                printf(
                        "Context item [%s] does not contain a supported dealloc type.\n", 
                        context_item->name
                );
            break;
        }
    }

}

void serve(HTTPServer* server) {
    int server_socket, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    int reuse_opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt)) < 0) {
        perror("Failed to setsockopt");
    }

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Failed to bind server");
        exit(1);
    }
    if (listen(server_socket, 3) < 0) {
        perror("Failed to listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        char buffer[BUFFER_SIZE] = {};
        size_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    
        HTTPResponse client_resp = bufftoresp(buffer, bytes_read);
        printf("socket ptr -> %p\n", &client_socket);
        Context context = {};
        Response server_resp = eval_events(server, &client_resp, &client_socket, &context);
        
        // char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";
        
        send_resp(client_socket, server_resp, &context);
        free_context(&context);
        close(client_socket);
    }
}


#endif

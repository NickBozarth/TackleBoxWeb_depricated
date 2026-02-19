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


int servefile(SOCKET clientSocket, const char *filePath) {
    FILE* fp;
    char fullFilePath[sizeof(SERVE_DIRECTORY) + MAX_FILENAME_LEN];
    snprintf(fullFilePath, sizeof(fullFilePath), "%s%s", SERVE_DIRECTORY, filePath);

    fp = fopen(fullFilePath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open file [%s]\n", fullFilePath);
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
        "Content-Type: text/plain\r\n"
        "Content-Length: %li\r\n"
        "\r\n",

        fileSize
    );

    send_client(clientSocket, header, headerLen);

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send_client(clientSocket, buffer, bytesRead);
    }

    fclose(fp);
    return 0;
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
    
        HTTPResponse resp = bufftoresp(buffer, bytes_read);
        printf("socket ptr -> %p\n", &client_socket);
        EventResponse eres = eval_events(server, &resp, &client_socket);
        if (eres.type == EVENT_RESPONSE_ERR) printf("EVENTS ERR %i\n", eres.data.err);
        else printf("EVENTS RETURNED {\n%s\n}", eres.data.resp.body);

        char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";

        printf("response -> %s\n", response);
        // write(client_socket, response, strlen(response));
        servefile(client_socket, "index.html");
        close(client_socket);
    }
}


#endif

#ifndef _WIN32

#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void serve(HTTPServer* server) {
    int server_socket, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_socket, (struct sockaddr*)&address, sizeof(address));
    listen(server_socket, 3);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&address, (socklen_t*)&addrlen);

        char buffer[BUFFER_SIZE] = {};
        size_t bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    
        HTTPResponse resp = bufftoresp(buffer, bytes_read);
        char* event_response = eval_events(server, &resp);
        if (event_response == NULL) printf("EVENTS RETURNED NULL\n");
        else printf("EVENTS RETURNED {\n%s\n}", event_response);

        char *response = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello World!";
        write(client_socket, response, strlen(response));
        close(client_socket);
    }
}


#endif

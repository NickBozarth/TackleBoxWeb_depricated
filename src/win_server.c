// all definitions for windows server functions
#ifdef _WIN32

#include "server.h"
#include "http.h"

const char* gFileNotFoundError = "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found";



int servefile(SOCKET clientSocket, const char* filePath) {
    FILE* fp;
    char fullFilePath[sizeof(SERVE_DIRECTORY) + MAX_FILENAME_LEN] = SERVE_DIRECTORY;
    strcat(fullFilePath, filePath); // TODO switch to a safe version: ex snprintf or strncat

    fp = fopen(fullFilePath, "rb");
    if (fp == NULL) {
        printf("Failed to open file %s", fullFilePath);
        perror("");
        send(clientSocket, gFileNotFoundError, (int)strlen(gFileNotFoundError), 0);
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[512];
    int headerLen = snprintf(
        header, 
        sizeof(header), 
        HTTP_HEADER_OK HTTP_HEADER_TEXT_HTML HTTP_HEADER_CONTENT_LENGTH_UNFORMAT HTTP_HEADER_CLOSE HTTP_HEADER_END,
        fileSize
    );
    send(clientSocket, header, headerLen, 0);

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send(clientSocket, buffer, (int)bytesRead, 0);
    }

    fclose(fp);
    return 0;
}


void print_buffer_bytes(const char* buffer, const long buffer_size) {
    for (long i = 0; i < buffer_size; i++) {
        if (i % 16 == 0) printf("\n");
        else if (i % 8 == 0) printf("\t");
        else if (i % 4 == 0) printf(" ");
        printf("%02x ", buffer[i]);
    }
}


void readheaders(char* recieve_buffer, const long recieved_bytes) {
    print_buffer_bytes(recieve_buffer, recieved_bytes);

    char* line;
    char* state;
    char* body_start = strstr(recieve_buffer, "\r\n\r\n");
    if (body_start) {
        *body_start = '\0';
        body_start += 4;
    }

    printf("HEADER END 111 %p\n", body_start);

    line = strtok_r(recieve_buffer, " ", &state);
    printf("LINE [[%s]]\n", line);
    line = strtok_r(NULL, " ", &state);
    printf("LINE [[%s]]\n", line);
    line = strtok_r(NULL, "\r\n", &state);
    printf("LINE [[%s]]\n", line);



    while(
        (line = strtok_r(NULL, "\r\n", &state)) != NULL
    ) {
        if (line >= body_start) break;

        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';

            char* key = line;
            char* value = colon + 1;

            while (*value == ' ') value++;

            printf("%p - Header: [%s] -> Value: [%s]\n", line, key, value);
        }
    }

    printf("LINE %s\n", body_start);
}


int openserver() {
    WSADATA wsaData;
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    char recieveBuffer[BUFFER_SIZE];

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return 1;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, PORT, &hints, &result);

    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
    freeaddrinfo(result);

    listen(listenSocket, SOMAXCONN);
    printf("Server running on port %s...\n", PORT);

    while (1) {
        clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) continue;

        int recievedBytes = recv(clientSocket, recieveBuffer, BUFFER_SIZE, 0);
        if (recievedBytes > 0) {
            if (recievedBytes >= BUFFER_SIZE) recieveBuffer[BUFFER_SIZE-1] = '\0';
            recieveBuffer[recievedBytes] = '\0';

            bufftoresp(recieveBuffer, recievedBytes);

            int err;
            if ((err = servefile(clientSocket, "index.html")) > 0) {
                printf("err %i\n", err);
            }
        }



        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}

#endif

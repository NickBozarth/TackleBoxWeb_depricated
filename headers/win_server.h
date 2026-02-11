#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdio.h>

#define PORT "8080"
#define SERVE_DIRECTORY ".\\serve\\"
#define MAX_FILENAME_LEN 256
#define BUFFER_SIZE 1024

#define HTTP_HEADER_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_HEADER_TEXT_PLAIN "Content-Type: text/plain\r\n"
#define HTTP_HEADER_TEXT_HTML "Content-Type: text/html\r\n"
#define HTTP_HEADER_CONTENT_LENGTH_UNFORMAT "Content-Length: %ld\r\n"
#define HTTP_HEADER_KEEP_ALIVE "Connection: keep-alive\r\n"
#define HTTP_HEADER_CLOSE "Connection: close\r\n"
#define HTTP_HEADER_END "\r\n"



int servefile(SOCKET clientSocket, const char* filePath);
void print_buffer_bytes(const char* buffer, const long buffer_size);
int openserver();



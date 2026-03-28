#include <unistd.h>
#include <poll.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    // creating socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("socket");
        return 1;
    }

    // specifying address
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    // connection
    if (connect(clientSocket, (struct sockaddr*)&serverAddress,
                sizeof(serverAddress)) < 0)
    {
        perror("connect");
        return 1;
    }

    // sending request
    const char* message = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r";
    send(clientSocket, message, strlen(message), 0);

    // receiving response
    char buffer[4096];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0'; // null-terminate
        std::cout << buffer;
    }

    if (bytesRead < 0)
        perror("recv");

    // closing socket
    safeClose(clientSocket);

    return 0;
}
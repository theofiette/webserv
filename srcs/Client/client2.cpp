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
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // sending connection request
    if (connect(clientSocket, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress)) < 0)
    {
        perror("connect");
        close(clientSocket);
        return 1;
    }

    // sending data
    const char* message = "GET /permissions.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r";

    std::cout << "----- Requete envoyee -----\n";
    std::cout << message << std::endl;

    ssize_t sent = send(clientSocket, message, strlen(message), 0);
    if (sent < 0)
    {
        perror("send");
        close(clientSocket);
        return 1;
    }

    std::cout << "----- Reponse recue -----\n";
    char buffer[4096];
    ssize_t bytesRead = 0;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesRead] = '\0';
        std::cout << buffer;
    }

    if (bytesRead < 0)
        perror("recv");
    else
        std::cout << std::endl;

    // closing socket
    close(clientSocket);

    return 0;
}
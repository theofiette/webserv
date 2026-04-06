#include <unistd.h>
#include <poll.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

// fonction de fermeture sécurisée
void safeClose(int fd)
{
    if (fd >= 0)
        close(fd);
}

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("socket");
        return 1;
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("connect");
        safeClose(clientSocket);
        return 1;
    }

    const char* headers =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n\r\n";

    send(clientSocket, headers, strlen(headers), 0);

    const size_t CHUNK_SIZE = 8192;
    std::string bigChunk(CHUNK_SIZE, 'A');

    std::string requestChunk;

    char sizeBuffer[32];
    snprintf(sizeBuffer, sizeof(sizeBuffer), "%zx\r\n", bigChunk.size());

    requestChunk.append(sizeBuffer);
    requestChunk.append(bigChunk);
    requestChunk.append("\r\n");

    send(clientSocket, requestChunk.c_str(), requestChunk.size(), 0);

    std::string bigChunk2(CHUNK_SIZE, 'B');

    requestChunk.clear();
    snprintf(sizeBuffer, sizeof(sizeBuffer), "%zx\r\n", bigChunk2.size());

    requestChunk.append(sizeBuffer);
    requestChunk.append(bigChunk2);
    requestChunk.append("\r\n");

    send(clientSocket, requestChunk.c_str(), requestChunk.size(), 0);

    send(clientSocket, "0\r\n\r\n", 5, 0);

    char recvBuffer[4096];
    int bytesRead;
    while ((bytesRead = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0)
    {
        recvBuffer[bytesRead] = '\0';
        std::cout << recvBuffer;
    }

    if (bytesRead < 0)
        perror("recv");

    safeClose(clientSocket);
    return 0;
}
#include <unistd.h>
#include <poll.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void safeClose(int fd)
{
    if (fd >= 0)
        close(fd);
}

void sendChunk(int socket, const std::string &data)
{
    char sizeBuffer[32];
    snprintf(sizeBuffer, sizeof(sizeBuffer), "%zx\r\n", data.size());

    std::string chunk;
    chunk.append(sizeBuffer);
    chunk.append(data);
    chunk.append("\r\n");

    send(socket, chunk.c_str(), chunk.size(), 0);
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

    const size_t SMALL_CHUNK_SIZE = 512;

    const size_t TOTAL_SIZE = 16384;

    size_t sent = 0;

    while (sent < TOTAL_SIZE)
    {
        size_t currentSize = std::min(SMALL_CHUNK_SIZE, TOTAL_SIZE - sent);

        std::string chunkData(currentSize, (sent < TOTAL_SIZE / 2) ? 'A' : 'B');

        sendChunk(clientSocket, chunkData);

        sent += currentSize;
    }

    // chunk final
    send(clientSocket, "0\r\n\r\n", 5, 0);

    // réception
    char recvBuffer[4096];
    int bytesRead;
    while ((bytesRead = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0)
    {
        recvBuffer[bytesRead] = '\0';
        // std::cout << recvBuffer;
    }

    if (bytesRead < 0)
        perror("recv");

    safeClose(clientSocket);
    return 0;
}
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
    // création du socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        perror("socket");
        return 1;
    }

    // configuration de l'adresse du serveur
    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    // connexion
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("connect");
        safeClose(clientSocket);
        return 1;
    }

    // création de la requête POST chunked
    const char* headers =
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Content-Type: text/plain\r\n"
        "Connection: close\r\n\r\n";

    send(clientSocket, headers, strlen(headers), 0);

    // envoi de chunks
    const char* chunk1 = "Hello, ";
    const char* chunk2 = "this is chunked data!";
    
    char buffer[128];

    // chunk 1
    snprintf(buffer, sizeof(buffer), "%x\r\n%s\r\n", (int)strlen(chunk1), chunk1);
    send(clientSocket, buffer, strlen(buffer), 0);

    // chunk 2
    snprintf(buffer, sizeof(buffer), "%x\r\n%s\r\n", (int)strlen(chunk2), chunk2);
    send(clientSocket, buffer, strlen(buffer), 0);

    // chunk final (0-length)
    send(clientSocket, "0\r\n\r\n", 5, 0);

    // réception de la réponse
    char recvBuffer[4096];
    int bytesRead;
    while ((bytesRead = recv(clientSocket, recvBuffer, sizeof(recvBuffer) - 1, 0)) > 0)
    {
        recvBuffer[bytesRead] = '\0';
        std::cout << recvBuffer;
    }

    if (bytesRead < 0)
        perror("recv");

    // fermeture du socket
    safeClose(clientSocket);

    return 0;
}
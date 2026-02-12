#include <unistd.h>
#include <poll.h>
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <cstring>


#define LOCALHOST	"8080"

template< typename T>
void	ft_bzero(T *ptr, const size_t size)
{
	char *ptr_ref = reinterpret_cast<char *>(ptr);

	for (size_t i = 0; i < size; i++)
		ptr_ref[i] = 0;
}

int	initMasterSocket()
{
	int 			masterSocket;
	struct addrinfo *gai;
    
	
	struct addrinfo hints;

	ft_bzero< struct addrinfo >(&hints, sizeof(hints));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */


    //    hints.ai_protocol = 0;          /* Any protocol */
    //    hints.ai_canonname = NULL;
    //    hints.ai_addr = NULL;
    //    hints.ai_next = NULL;

	// socket gives us the masterSocket FD
	masterSocket = socket(AF_INET, SOCK_STREAM, 0);

	// we fill a sockaddr structure
	getaddrinfo(NULL, LOCALHOST, &hints, &gai);
	// sockaddr_in serverAddress;
	// serverAddress.sin_family = AF_INET;
	// serverAddress.sin_addr.s_addr = INADDR_ANY;
	// serverAddress.sin_port = htons(LOCALHOST);

	bind(masterSocket, gai->ai_addr, gai->ai_addrlen);
	listen(masterSocket, 1024);

	return (masterSocket);
}

int main()
{
	int masterSocket = initMasterSocket();

	pollfd pollfds[10];
	while(1)
	{
		
		int i = 0;
		
		poll(pollfds, 10, 0);

		pollfds[i].fd = accept(masterSocket, 0, 0);

		char buffer[2000] = {0};

		recv(pollfds[i].fd, buffer, sizeof(buffer), 0);

		std::cout << buffer << "\n";
		i++;
	}
	
	close(masterSocket);
}	
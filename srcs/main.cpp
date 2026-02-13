// #include "../includes/webserv.hpp"
#include "webserv.hpp"
#include <vector>

#define LOCALHOST	"8080"
#define MAX_CLIENTS	1024

int	initMasterSocket()
{
	int 			masterSocket;
	struct addrinfo *gai;
	struct addrinfo hints;

	t_bzero< struct addrinfo >(&hints, sizeof(hints));
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
	listen(masterSocket, MAX_CLIENTS);

	return (masterSocket);
}

int main()
{
	int masterSocket = initMasterSocket();
	
	pollfd pollfds[MAX_CLIENTS];
	t_bzero(pollfds, sizeof(pollfds));
	pollfds[0].fd = masterSocket;
	pollfds[0].events = POLLIN;


	// std::vector<pollfd> pollfds;
	// pollfds.reserve(MAX_CLIENTS);


	while(POLLIN)
	{
		poll(pollfds, MAX_CLIENTS, 0);
		std::cout << "OUT OF POLL" << std::endl;


		// poll(pollfds, 10, 0);
		// for (int i = 0; i < MAX_CLIENTS; i++)
		// {
		// 	// check if should accept
		// 	pollfds[i].events = POLLIN;
		// 	pollfds[i].fd = accept(masterSocket, 0, 0);
		// 	if (pollfds[i].revents | POLLIN)
		// 	{
		// 		char buffer[2000] = {0};
				
		// 		recv(pollfds[i].fd, buffer, sizeof(buffer), 0);
				
		// 		std::cout << buffer << "\n";
		// 	}
		// }
	}
	
	close(masterSocket);
}

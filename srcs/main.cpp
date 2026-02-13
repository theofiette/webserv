// #include "../includes/webserv.hpp"
#include "webserv.hpp"
#include <vector>

#define LOCALHOST	"8080"
#define MAX_CLIENTS	1024
#define TIMEOUT		(3 * 60 * 1000)

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
	fcntl(masterSocket, FNDELAY);

	return (masterSocket);
}

int main()
{
	int masterSocket = initMasterSocket();

	pollfd pollfds[MAX_CLIENTS];
	t_bzero(pollfds, sizeof(pollfds));
	pollfds[0].fd = masterSocket;
	pollfds[0].events = POLLIN;

	int	socketCount = 1;
	while(1)
	{
		int nfds = socketCount;
		int err = poll(pollfds, nfds, TIMEOUT); // TODO check err
		(void)err;
		std::cout << "OUT OF POLL" << std::endl;
		std::cout << "nfds -> " << nfds << std::endl;

		for (int i = 0; i < nfds; i++)
		{
			pollfd	curr_request = pollfds[i];
			std::cout <<  "fd is " << curr_request.fd << "\n";
			if (curr_request.revents == 0)
				continue ;
			if (curr_request.revents == POLLIN) // THERE IS AN EVENT HERE
			{
				if (curr_request.fd == masterSocket) // <-- REQUEST COMES FROM MASTER SOCKET
				{
					std::cout << "Request coming from server\n";
					while (int fd = accept(masterSocket, 0, 0)) // WHY O and 0 in accept  ??
					{
						if (fd == -1) // error ?
						{
							std::cout << "accept returned -1\n";

							// DEAL ERRORS WITH ERRNO
							break ;
						}
						std::cout << "err2\n";

						pollfds[socketCount].fd = fd;
						pollfds[socketCount].events = POLLIN;
						socketCount++;
					}
				}
				else if (curr_request.fd != masterSocket) // <-- REQUEST COMES FROM CLIENT
				{
					std::cout << "Request coming from client\n";
					char buffer[2000] = {0};
					recv(curr_request.fd, buffer, sizeof(buffer), 0);
					std::cout << buffer << "\n";
				}
			}
		}
	}
	
	close(masterSocket);
}

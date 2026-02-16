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
	// int on = 1;
	// setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
	int val = fcntl(masterSocket, F_SETFL, O_NONBLOCK);
	if (val == -1)
		std::cout << "ERROR ON FCNTL \n\n";
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
	int nfds = 1;
	int newSocket = 0;
	int errCheck = 0;
	int factoArray = 0;

	while(!errCheck)
	{
		// std::cout << "NFDS = " << nfds << "\n\n";
		// std::cout << "POLLFDS[0] == MASTERSOCKET " << (pollfds[0].fd == masterSocket ? "TRUE" : "FALSE") << "\n";
		poll(pollfds, nfds, 3 * 60 * 1000);
		// std::cout << "OUT OF POLL" << std::endl;

		
		int currentSize = nfds;
		factoArray = 0;
		for (int i = 0; i < currentSize; i++)
		{
			if (!pollfds[i].revents)
				continue;

			if (pollfds[i].revents != POLLIN)
				break;

			if (pollfds[i].fd == masterSocket){

				while(1){
					newSocket = accept(masterSocket, 0, 0);
					if (newSocket < 0){
						break ;
					}
					pollfds[nfds].fd = newSocket; 
					pollfds[nfds].events = POLLIN;
				}
				nfds++;
			}
			else{
				char buffer[2000] = {0};
				std::string buf;
				std::cout << pollfds[i].fd << "\n";
				while (int rc = recv(pollfds[i].fd, buffer, sizeof(buffer) - 1, 0) > 0){
					buf += buffer;
					t_bzero(&buffer, sizeof(buffer));
					std::cout << "BYTES READ : " << rc << "\n";
				}
				std::cout << "MESSAGE IS : " << buf << "\n";
				close(pollfds[i].fd);
				pollfds[i].fd = -1;
				factoArray = 1;

			}
		}
		if (factoArray){
			for (int i = 0; i < nfds; i++){
				if (pollfds[i].fd == -1){
					for (int j = i; j < nfds - 1; j++){
						pollfds[j] = pollfds[j + 1];
					}
					i--;
					nfds--;
				}
			}
		}
	}
	close(masterSocket);
}

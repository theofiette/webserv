#include "parser.hpp"

static std::string recvRequest(int socketFd)
{
	std::cout << "init request...\n";
	std::string request;
	char buffer[2000] = {0};
	while (int rc = recv(socketFd, buffer, sizeof(buffer) - 1, 0) > 0)
	{
		request += buffer;
		t_bzero(&buffer, sizeof(buffer));
	}
	std::cout << request << std::endl;
	return (request);
}
static int parseRequest(std::string request)
{
	// int i = 0;
	// while (request[i] && request[i] != '\n')
	// {
		std::cout << request << '\n';
	// 	i++;
	// }
	return (1);
}

Request::Request(int &socketFd) : _requestType(""), _target("")
{
	// pid_t fd = fork();
	// if (fd == 0)
	// {
	std::string request = recvRequest(socketFd);
	// std::cout << request << '\n';

	parseRequest(request);
	close(socketFd);
	socketFd = -1;
	exit(0);
	// }

}

Request::~Request(){std::cout << "request ended\n";};
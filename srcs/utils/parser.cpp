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
	return (request);
}

void Request::setRequestType(int type)
{
	if (type == GET)
		_requestType = GET;
	else if (type == DELETE)
		_requestType = DELETE;
	else if (type == POST)
		_requestType = POST;
	else 
		_requestType = INVALID;
}

void Request::setTarget(std::string target)
{
	if (!target.empty())
		_target = target;
}

void Request::setProtocole(std::string protocole)
{
	std::string("HTTP/1.1");

	// if ()
	_protocole = protocole;
}

static int checkType(std::string request)
{
	std::string type[3] = {"GET", "DELETE", "POST"};

	for (int i = 0; i < 3; i++)
	{
		if (request.find(type[i]) != std::string::npos)
			return (i);
	}
	return (INVALID);
}

static int parseRequest(Request &r, std::string request)
{
	std::string tmp;

	tmp = request.substr(0, request.find(' '));
	std::cout << tmp << std::endl;

	int type = checkType(tmp);
	r.setRequestType(type);
	
	std::cout << type << std::endl;

	tmp = request.substr(tmp.size() + 1, request.find(' ') - 1);
	r.setTarget(tmp);
	
	tmp = request.substr(tmp.size() + 1, request.find('\n'));
	r.setProtocole(tmp);
	std::cout << tmp << std::endl;

	return (1);
}

Request::Request(int &socketFd) : _requestType(INVALID), _target("")
{
	// pid_t fd = fork();
	// if (fd == 0)
	// {
		std::string request = recvRequest(socketFd);
		// std::cout << request << '\n';
		parseRequest(*this, request);
		close(socketFd);
		socketFd = -1;
	// 	exit(0);
	// }

}

Request::~Request(){std::cout << "request ended\n";};
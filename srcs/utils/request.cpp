#include "request.hpp"

static std::string recvRequest(int socketFd)
{
	std::cout << "init request...\n";
	char buffer[2000] = {0};
	std::string str;

	while (recv(socketFd, buffer, sizeof(buffer) - 1, 0))
	{
		// TODO : check error recv
		str += buffer;
		t_bzero(&buffer, sizeof(buffer));
		if (str.find("\r\n\r\n") == std::string::npos)
			continue ;
		else
			break ;
	}
	// std::cout << str << "\n";
	return (str);
}

void Request::setRequestType(int type)
{
	_requestType = type;
}

void Request::setTarget(std::string target)
{
	if (!target.empty())
		_target = target;
}

void Request::setProtocole(std::string protocole)
{
	// if (protocole == "HTTP/1.1")
	_protocole = protocole;
	// else
	// 	std::cout << "ERROR ON PROTOCOL\n";
}

static int checkType(std::string request)
{
	const std::string type[3] = {"GET", "DELETE", "POST"}; // recree a chaque fois ?

	for (int i = 0; i < 3; i++)
		if (request.find(type[i]) != std::string::npos)
			return (i);
	return (Request::INVALID);
}

static int parseRequest(Request &r, std::string request)
{
	std::string tmp;
	size_t offset = 0;

	std::cout << request << std::endl;
	tmp = request.substr(0, request.find(' '));
	offset = tmp.size() + 1;

	int type = checkType(tmp);
	r.setRequestType(type);
	tmp = request.substr(offset, request.find(' ', offset) - offset);
	r.setTarget(tmp);
	
	offset += tmp.size() + 1;
	
	tmp = request.substr(offset, request.find('\n') - offset);
	r.setProtocole(tmp);

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
		if (this->_requestType == GET)
		{
			char msg[500] = "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: 2000\n\n<h1> ILIES IL EST BO DES FOIS <h1/> <br> <h2> JE SUIS UN SOUS TITRE A LA LIGNE <h2/> <br> <p>ET JE SUIS UN PARAGRAPHE EN GRAS<p/>\n";
			send(socketFd, msg, sizeof(msg), 0);
		}
		close(socketFd);
		socketFd = -1;
	// 	exit(0);
	// }

}

Request::~Request(){std::cout << "request ended" << std::endl;};
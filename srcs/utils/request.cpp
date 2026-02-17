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
	std::cout << str << "\n";
	return (str);
}

void Request::displayInfos(){
	std::cout << "\n--------\nMethod : " << _requestType << "\n"
		<< "Path :" << _target << "\n" << "Protocol : " << _protocole << "\nHost : " << _host << "\nPort : " << _port
		<< "\nConnectionType : " << _connectionType << "\n--------\n";   
}

void Request::setRequestType(int type)
{
	if (type == -1)
		throw ExceptionParsingError();
	_requestType = type;
}

void Request::setConnectionType(std::string connectionType)
{
	_connectionType = connectionType;
}

void Request::setTarget(std::string target)
{
	if (target.find('/') == std::string::npos)
		throw ExceptionParsingError();
	if (!target.empty())
		_target = target;
}

void Request::setProtocole(std::string protocole)
{
	if (protocole == "HTTP/1.0\r" || protocole == "HTTP/1.1\r")
	{
		protocole[protocole.size() - 1] = 0;
		_protocole = protocole;
	}
	else
		throw ExceptionParsingError();
}

void Request::setHostPort(std::string host)
{
	if ((host.find(':') == std::string::npos))
		throw ExceptionParsingError();
	std::string port = host.substr(host.find(':') + 1, host.size() - 1);
	if (port.find_first_not_of("0123456789") != std::string::npos)
		throw ExceptionParsingError();
	_host = host.substr(0, host.find(':'));
	_port = port; // TODO: to end
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

	try {
		tmp = request.substr(0, request.find(' '));
		offset = tmp.size() + 1;
		int type = checkType(tmp);
		r.setRequestType(type);

		tmp = request.substr(offset, request.find(' ', offset) - offset);
		r.setTarget(tmp);

		offset += tmp.size() + 1;

		tmp = request.substr(offset, request.find('\n') - offset);
		r.setProtocole(tmp);
		
		size_t hostPos = request.find("Host: ");
		if (hostPos == std::string::npos)
			throw ExceptionParsingError();
		hostPos += 6;		
		tmp = request.substr(hostPos, request.find("\r\n", hostPos) - hostPos);
		r.setHostPort(tmp);

		size_t connectionPos = request.find("Connection: ");
		if (connectionPos == std::string::npos)
			throw ExceptionParsingError();
		connectionPos += 12;
		tmp = request.substr(connectionPos , request.find('\n', connectionPos) - connectionPos - 1);
		r.setConnectionType(tmp);
	}
	catch (std::exception &e){
		std::cout << "EXCEPTION IS: " <<  e.what() << "\n";
	}
	return (1);
}

void Request::createReponse()
{
	;
}

void Request::rGet()
{
	;
}

void Request::rDelete()
{
	;
}

void Request::rPost()
{
	;
}


void Request::pickRequestType()
{
	switch (_requestType)
	{
		case GET:
			rGet();
			break;
		case DELETE:
			rDelete();
			break;
		case POST:
			rPost();
			break;
		default:
			std::cout << "INVALID REQUEST TYPE\n";
			break;
	}
}

Request::Request(int &socketFd) : _requestType(INVALID), _target("")
{
	std::string request = recvRequest(socketFd);
	// std::cout << request << '\n';
	parseRequest(*this, request);
	displayInfos();
	// if (this->_requestType == GET)
	// {
		char msg[500] = "HTTP/1.0 404 Not found\nContent-Type:text/html\nContent-Length: 2000\n\n<h1> ILIES IL EST BO DES FOIS <h1/> <br> <h2> JE SUIS UN SOUS TITRE A LA LIGNE <h2/> <br> <p>ET JE SUIS UN PARAGRAPHE EN GRAS<p/>\n";
		send(socketFd, msg, sizeof(msg), 0);
	// }
	close(socketFd);
	socketFd = -1;
}

Request::~Request(){std::cout << "request ended" << std::endl;};
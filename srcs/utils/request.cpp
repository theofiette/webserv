#include "request.hpp"
#include <map>
#include <sstream>

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
	std::map<std::string, std::string>::iterator it = _reqMap.begin();
	for (; it != _reqMap.end(); it++)
		std::cout << it->first << " : " << it->second << "\n";
}

// void Request::setRequestType(int type)
// {
// 	// if (type == -1)
// 	// 	throw ExceptionParsingError();
// 	// _requestType = type;
// }

// void Request::setConnectionType(std::string connectionType)
// {
// 	// _connectionType = connectionType;
// }

// // void Request::setTarget(std::string target)
// // {
// // 	if (target.find('/') == std::string::npos)
// // 		throw ExceptionParsingError();
// // 	if (!target.empty())
// // 		_target = target;
// // }

// void Request::setProtocole(std::string protocole)
// {
// 	// if (protocole == "HTTP/1.0\r" || protocole == "HTTP/1.1\r")
// 	// {
// 	// 	protocole[protocole.size() - 1] = 0;
// 	// 	_protocole = protocole;
// 	// }
// 	// else
// 	// 	throw ExceptionParsingError();
// }

// void Request::setHostPort(std::string host)
// {
// 	// if ((host.find(':') == std::string::npos))
// 	// 	throw ExceptionParsingError();
// 	// std::string port = host.substr(host.find(':') + 1, host.size() - 1);
// 	// if (port.find_first_not_of("0123456789") != std::string::npos)
// 	// 	throw ExceptionParsingError();
// 	// _host = host.substr(0, host.find(':'));
// 	// _port = port; // TODO: to end
// }

// static int checkType(std::string request)
// {
// 	const std::string type[3] = {"GET", "DELETE", "POST"}; // recree a chaque fois ?

// 	for (int i = 0; i < 3; i++)
// 		if (request.find(type[i]) != std::string::npos)
// 			return (i);
// 	return (Request::INVALID);
// }

void Request::setFirstLine(std::string tmp){
    std::istringstream iss(tmp);
    std::string method, path, protocol;
	if (!(iss >> method >> path >> protocol)) {
		throw ExceptionParsingError();
	}
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
		protocol[protocol.size() - 1] = 0;
    _reqMap["Method"] = method;
    _reqMap["Path"] = path;
    _reqMap["Protocol"] = protocol;
}

int Request::parseRequest(std::string request)
{
	std::string tmp;
	std::stringstream reqF(request, std::ios_base::in);

	try {
		std::getline(reqF, tmp, '\n');
		setFirstLine(tmp);
		while (std::getline(reqF, tmp, '\n')){
			if (tmp.empty() || tmp[0] == '\r')
				break;
			std::pair<std::string, std::string> map;
			size_t sep = tmp.find(": ");
			if (sep == std::string::npos)
				throw ExceptionParsingError();
			map.first = tmp.substr(0, sep);
			size_t end = tmp.find('\r');
			if (end == std::string::npos)
				throw ExceptionParsingError();
			sep += 2; // SKIP ": " 
			map.second = tmp.substr(sep, end - sep);
			_reqMap.insert(map);
		}
		displayInfos();
	}
	catch (std::exception &e){
		std::cout << "EXCEPTION IS: " <<  e.what() << "\n";
	}
	return (1);
}

std::string Request::buildReponse()
{
	std::stringstream response; 

	response << _reqMap.at("Protocol") + " " + _responseCode + " OK\r\n";
	response << "Server: ";
	response << "Date: ";
	response << "Content-type: ";
	response << "Content-Length: ";
	response << "Connection: " << _reqMap.at("Connection") << "\r\n";

	return (response.str());
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

Request::Request(int &socketFd) : _requestType(INVALID)
{
	std::string request = recvRequest(socketFd);
	// std::cout << request << '\n';
	parseRequest(request);
	// displayInfos();
	// if (this->_requestType == GET)
	// {
		char msg[500] = "HTTP/1.0 404 Not found\nContent-Type:text/html\nContent-Length: 2000\n\n<h1> ILIES IL EST BO DES FOIS <h1/> <br> <h2> JE SUIS UN SOUS TITRE A LA LIGNE <h2/> <br> <p>ET JE SUIS UN PARAGRAPHE EN GRAS<p/>\n";
		send(socketFd, msg, sizeof(msg), 0);
	// }
	close(socketFd);
	socketFd = -1;
}

Request::~Request(){std::cout << "request ended" << std::endl;};
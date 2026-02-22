#include "webserv.hpp"
#include "Request.hpp"
#include <map>
#include <sstream>

static std::string recvRequest(int socketFd)
{
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
	return (str);
}

void Request::displayInfos(){
	std::map<std::string, std::string>::iterator it = _reqMap.begin();
	for (; it != _reqMap.end(); it++)
		std::cout << it->first << " : " << it->second << "\n";
}


void Request::setFirstLine(std::string tmp)
{
    std::istringstream iss(tmp);
    std::string method, path, protocol;

	if (!(iss >> method >> path >> protocol)) 
		throw ExceptionParsingError();
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
		while (std::getline(reqF, tmp, '\n'))
		{
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

std::map<std::string, std::string>& Request::getMap()
{
	return (_reqMap);
}

Request::Request(int &socketFd)
{
	std::string request = recvRequest(socketFd);
	parseRequest(request);
}

Request::~Request(){};
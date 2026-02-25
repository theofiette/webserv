#include "webserv.hpp"
#include "Request.hpp"
#include <map>
#include <sstream>

Request::Request(Server &server) : _totalBytes(0), _requestComplete(false), _currServ(server) {}

Request::~Request(){}

static int stringToInt(std::string num)
{
    std::stringstream ss(num);
    int value;
	ss >> value;
    return (value);
}

static std::string extractContentLength(std::string buffer)
{
	std::string contentLength;
	size_t pos = buffer.find("Content-length : ");
	if (pos != std::string::npos)
	{
		size_t end = buffer.find('\r');
		if (end != std::string::npos)
			contentLength = buffer.substr(pos, end - pos);
		else
			throw ExceptionParsingError();
	}
	else
	{
		return ("-1");
	}
	return (contentLength);
}

void Request::recvRequest(int socketFd)
{
	char buffer[2000] = {0};

	ssize_t bytesRead = recv(socketFd, buffer, sizeof(buffer) - 1, 0);
	std::cout << "Total bytes = " << _totalBytes << std::endl;
	_totalBytes += bytesRead;
	_rawRequest += buffer;
	std::cout << "Buffer: " << _rawRequest << std::endl;
	t_bzero(&buffer, sizeof(buffer));
	if (_rawRequest.find("\r\n\r\n") != std::string::npos)
	{
		int contentLength = stringToInt(extractContentLength(_rawRequest));
		if (contentLength < 0)
			_requestComplete = true;
		else if (contentLength > 0)
		{
			if (contentLength > _currServ.getMaxBody())
			{
				_requestComplete = true;
				return ;
			}
			// check MAX_BODY_SIZE avec TOTAL BYTES et QUAND END, TOTALBYTES = CONTENT LENGTH + REQUEST LENTGHT SINON ON A TROP LU 
			// ;
		}
		else if (contentLength == 0)
		{
			_requestComplete = true;
		}
		// TODO: get body
	}
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
		// displayInfos();
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

std::string &Request::getRawRequest(){
	return (_rawRequest);
}

bool &Request::isRequestComplete(){
	return (_requestComplete);
}

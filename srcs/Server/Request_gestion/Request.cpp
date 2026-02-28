#include "webserv.hpp"
#include "Request.hpp"
#include <map>
#include <sstream>

Request::Request(Server &server) : _totalBytes(0), _requestComplete(false), _currServ(server) {}

Request::~Request(){}

int stringToInt(std::string num)
{
    std::stringstream ss(num);
    int value;
	ss >> value;
    return (value);
}

std::string extractContentLength(std::string buffer)
{
	std::string contentLength;
	size_t pos = buffer.find("Content-Length:") + 16;

	if (pos != std::string::npos)
	{
		size_t end = buffer.find("\r", pos);
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
	char buffer[SEND_SIZE] = {0};
	ssize_t bytesRead = recv(socketFd, buffer, SEND_SIZE - 1, 0);

	_totalBytes += bytesRead;
	_rawRequest += buffer;
	size_t headerEnd = _rawRequest.find(CRLF) + CRLF_SIZE;
	if (headerEnd == std::string::npos)
	{
		t_bzero(&buffer, sizeof(buffer));
		return ;
	}
	int contentLength = stringToInt(extractContentLength(_rawRequest));
	if (contentLength <= 0)
		_requestComplete = true;
	else if (contentLength > 0)
	{
		if (contentLength > _currServ.getMaxBody())
		{
			_requestComplete = true;
			return ;
		}
		_body += _rawRequest.substr(headerEnd + _body.size());
		if (_body.size() == static_cast<size_t>(contentLength))
		{
			std::cout << "BODY RECEIVED COMPLETE\n\n";
			_requestComplete = true;
			return ;
		}
	}
}

void Request::displayInfos() const
{
	std::map<std::string, std::string>::const_iterator it = _reqMap.begin();
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
	}
	catch (std::exception &e){
		std::cout << "EXCEPTION IS: " <<  e.what() << "\n";
	}
	return (1);
}

const std::string& Request::getBody() const
{
	return (_body);
}

const std::map<std::string, std::string>& Request::getMap() const
{
	return (_reqMap);
}

const std::string& Request::getRawRequest() const
{
	return (_rawRequest);
}

bool &Request::isRequestComplete()
{
	return (_requestComplete);
}

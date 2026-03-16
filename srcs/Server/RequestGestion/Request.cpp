#include "webserv.hpp"
#include "Request.hpp"
#include <map>
#include <sstream>

Request::Request(const Server &server) : _totalBytes(0), _requestSuccess(true), _requestComplete(false), _currServ(server) {}

Request::~Request(){}

int stringToInt(const std::string &num)
{
	std::stringstream ss(num);
	int value = -1;

	if (!(ss >> value))
		return (-1);
	return (value);
}

std::string extractContentLength(const std::string &buffer)
{
	std::string contentLength;
	size_t pos = buffer.find("Content-Length:");
	if (pos == std::string::npos)
		return ("-1");

	pos += 15;
	while (pos < buffer.size() && (buffer[pos] == ' ' || buffer[pos] == '\t'))
		pos++;

	size_t end = buffer.find("\r", pos);
	if (end == std::string::npos || end == pos)
		throw ExceptionParsingError();

	contentLength = buffer.substr(pos, end - pos);
	if (stringToInt(contentLength) < 0)
		throw ExceptionParsingError();

	return (contentLength);
}

ssize_t Request::readSocketChunk(int socketFd, char *buffer, size_t bufferSize)
{
	ssize_t bytesRead = recv(socketFd, buffer, bufferSize, 0);

	if (bytesRead < 0)
		throw ExceptionRuntime("recv() error");
	if (bytesRead == 0)
	{
		_requestComplete = true;
		return (0);
	}

	_totalBytes += bytesRead;
	_rawRequest.append(buffer, bytesRead);
	return (bytesRead);
}

bool Request::findBodyStart(size_t &bodyStart) const
{
	size_t headerEnd = _rawRequest.find(CRLFCRLF);

	if (headerEnd == std::string::npos)
		return (false);
	bodyStart = headerEnd + CRLF_SIZE;
	return (true);
}

bool Request::finalizeWithoutBody(int contentLength)
{
	if (contentLength <= 0)
	{
		_requestComplete = true;
		return (true);
	}
	if (contentLength > _currServ.getMaxBody()) // TODO: faier remonter lerror
	{
		_requestComplete = true;
		return (true);
	}
	return (false);
}

void Request::setBody(size_t bodyStart, int contentLength)
{
	if (_rawRequest.size() <= bodyStart)
	{
		_body.clear();
		return ;
	}
	if (_body.size() >= static_cast<size_t>(contentLength))
	{
		_body.erase(contentLength);
		_requestComplete = true;
		return ;
	}
	_body = _rawRequest.substr(bodyStart);
	
}


void Request::receiveRequestChunk(int socketFd)
{
	char buffer[SEND_SIZE] = {0};

	if (readSocketChunk(socketFd, buffer, SEND_SIZE) == 0)
		return ;

	size_t bodyStart = 0;
	if (findBodyStart(bodyStart) == false)
		return ;

	int contentLength = stringToInt(extractContentLength(_rawRequest));
	if (finalizeWithoutBody(contentLength) == true)
		return ;

	setBody(bodyStart, contentLength);
}

void Request::displayInfos() const
{
	std::map<std::string, std::string>::const_iterator it = _requestHeader.begin();
	for (; it != _requestHeader.end(); it++)
		std::cout << it->first << " : " << it->second << "\n";
}

bool Request::setFirstLine(std::string buffer)
{
    std::istringstream iss(buffer);
    std::string method, path, protocol;

	if (!(iss >> method >> path >> protocol)) 
		return (false);
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
		protocol[protocol.size() - 1] = 0;
    _requestHeader["Method"] = method;
    _requestHeader["Path"] = path;
    _requestHeader["Protocol"] = protocol; //TODO: check if correct ?? 

	return (true);
}

bool Request::fillHeader(std::stringstream &requestStream, std::string &buffer) 
{
	while (std::getline(requestStream, buffer, '\n'))
	{
		if (buffer.empty() || buffer[0] == '\r')
			break;
		std::pair<std::string, std::string> pair;
		size_t sep = buffer.find(": ");
		if (sep == std::string::npos)
		{
			_requestSuccess = false;
			return (false);
		}
		pair.first = buffer.substr(0, sep);
		size_t end = buffer.find('\r');
		if (end == std::string::npos)
		{
			_requestSuccess = false;
			return (false);
		}
		sep += 2;
		pair.second = buffer.substr(sep, end - sep);
		_requestHeader.insert(pair);
	}
	return (true);
}

void Request::stateRequestValidity()
{
	//TODO: if POST and no content length its invalid? 
	// if (_requestHeader["Protocol"] != "HTTP/1.0")
	// {
	// 	_requestSuccess = false;
	// 	return ;
	// }
	
	_requestSuccess = true;
}

void Request::parseRequest(std::string request)
{
	std::string buffer;
	std::stringstream requestStream(request, std::ios_base::in);

	std::getline(requestStream, buffer, '\n');
	if (setFirstLine(buffer) == false || fillHeader(requestStream, buffer) == false)
		return ;

	stateRequestValidity();
}

const Server &	Request::getServer() const
{
	return (_currServ);
}

const std::string& Request::getBody() const
{
	return (_body);
}

const std::map<std::string, std::string>& Request::getMap() const
{
	return (_requestHeader);
}

const std::string& Request::getRawRequest() const
{
	return (_rawRequest);
}

bool &Request::isRequestComplete()
{
	return (_requestComplete);
}

bool &Request::getRequestSuccess()
{
	return (_requestSuccess);
}

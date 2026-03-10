#include "webserv.hpp"
#include "Request.hpp"
#include <map>
#include <sstream>

Request::Request(const Server &server) : _totalBytes(0), _requestSuccess(true), _requestComplete(false), _currServ(server) {}

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
	size_t pos = buffer.find("Content-Length:");
	if (pos != std::string::npos)
	{
		pos += 16;
		size_t end = buffer.find("\r", pos);
		if (end != std::string::npos)
			contentLength = buffer.substr(pos, end - pos);
		else
			throw ExceptionRuntime("Didn't find CR in header, request invalid!\n"); // todo: fix 
	}
	else
		return ("-1");
	return (contentLength);
}

void Request::extractBody(int contentLength, size_t headerEnd)
{
	if (contentLength > _currServ.getMaxBody())
	{
		_requestComplete = true;
		return ;
	}
	_body += _rawRequest.substr(headerEnd + _body.size());
	if (_body.size() == static_cast<size_t>(contentLength))
	{
		_requestComplete = true;
		return ;
	}
}

void Request::recvRequest(int socketFd)
{
	char buffer[SEND_SIZE] = {0};
	ssize_t bytesRead = recv(socketFd, buffer, SEND_SIZE - 1, 0);

	if (bytesRead < 0)
		throw ExceptionRuntime("recv() error");

	_totalBytes += bytesRead;
	_rawRequest += buffer;
	size_t headerEnd = _rawRequest.find(CRLF) + CRLF_SIZE;
	if (headerEnd == std::string::npos)
	{
		t_bzero(&buffer, sizeof(buffer));
		return ;
	}
	int contentLength = stringToInt(extractContentLength(_rawRequest));  // TODO: verify that we have correctly took the content
	if (contentLength <= 0)
	{
		_requestComplete = true; // if i dont find content length and im not a get, it fucks 
		if (contentLength < 0)
			_requestSuccess = false;
	}
	else if (contentLength > 0)
		extractBody(contentLength, headerEnd);
}

void Request::displayInfos() const
{
	std::map<std::string, std::string>::const_iterator it = _requestHeader.begin();
	for (; it != _requestHeader.end(); it++)
		std::cout << it->first << " : " << it->second << "\n";
}

void Request::setFirstLine(std::string buffer)
{
    std::istringstream iss(buffer);
    std::string method, path, protocol;

	if (!(iss >> method >> path >> protocol))
	{
		_requestSuccess = false;
		return ;
	}
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
		protocol[protocol.size() - 1] = 0;
    _requestHeader["Method"] = method;
    _requestHeader["Path"] = path;
    _requestHeader["Protocol"] = protocol;
}


void Request::fillHeader(std::stringstream &requestStream, std::string &buffer) 
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
			return ;
		}
		pair.first = buffer.substr(0, sep);
		size_t end = buffer.find('\r');
		if (end == std::string::npos)
		{
			_requestSuccess = false;
			return ;
		}
		sep += 2;
		pair.second = buffer.substr(sep, end - sep);
		_requestHeader.insert(pair);
	}
}

bool Request::stateRequestValidity()
{
	return (getRequestSuccess());
}

void Request::parseRequest(std::string request)
{
	std::string buffer;
	std::stringstream requestStream(request, std::ios_base::in);

	std::getline(requestStream, buffer, '\n');
	setFirstLine(buffer);
	fillHeader(requestStream, buffer);
	if (getRequestSuccess() == false)
		return ;
	// _requestSuccess = true;
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

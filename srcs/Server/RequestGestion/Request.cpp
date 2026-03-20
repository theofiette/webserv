#include "webserv.hpp"
#include "Request.hpp"
#include "safemath.hpp"
#include <map>
#include <sstream>

Request::Request(const Server &server, unsigned int serverIndex) : 
_totalBytes(0),_headerComplete(false), _requestSuccess(true), _requestComplete(false), _requestTimedOut(false), _serverIndex(serverIndex), _currServ(server)
{
	_tracker = TimeoutTracker();
}

Request::~Request(){}

int extractContentLength(const std::string &buffer)
{
	int contentLength;
	size_t pos = buffer.find("Content-Length:");

	if (pos == std::string::npos)
		return (-1);

	pos += 15;
	while (pos < buffer.size() && (buffer[pos] == ' ' || buffer[pos] == '\t'))
		pos++;

	size_t end = buffer.find("\r", pos);
	if (end == std::string::npos || end == pos)
		return (-1);

	contentLength = stringToInt(buffer.substr(pos, end - pos));
	return (contentLength);
}

void Request::readSocketChunk(int socketFd, char *buffer, size_t bufferSize, size_t &headerEnd)
{
	ssize_t bytesRead = recv(socketFd, buffer, bufferSize, 0);

	if (bytesRead < 0)
		throw ExceptionRuntime("recv() error");
	if (bytesRead == 0)
	{
		_requestComplete = true;
		return ;
	}
	if (_headerComplete == false)
	{
		_header += buffer;
		headerEnd = findHeaderEnd(_header);
	}
	else
		_body += buffer;
	_totalBytes += bytesRead;
}

size_t Request::findHeaderEnd(std::string buffer)
{
	size_t headerEnd = buffer.find(CRLFCRLF);

	if (headerEnd == std::string::npos)
		return (-1);

	_headerComplete = true;
	return (headerEnd + CRLF_SIZE);
}

bool Request::finalizeWithoutBody(int contentLength)
{
	if (contentLength <= 0)
	{
		_requestComplete = true;
		return (true);
	}
	return (false);
}

void	Request::endRequestWithoutBody(const size_t& headerEnd) 
{
	if (headerEnd == _header.size())
		_requestComplete = true;
}

void	Request::storeBodyExceeding(const size_t &headerEnd)
{ 
	if (headerEnd != std::string::npos && headerEnd != _header.size())
	{
		_body = _header.substr(headerEnd);
		_header = _header.substr(0, headerEnd);
	}
}
//TODO: systeme diterations pour pas chercher la fin du header en boucle ? _iterations++ ? if _iterations < MAX_ITER
void Request::receiveRequestChunk(int socketFd)
{	
	char buffer[SEND_SIZE] = {0};
	size_t headerEnd = 0;

	if (_tracker.isExpired() == true)
	{
		_requestComplete = true;
		_requestSuccess = false;
		_requestTimedOut = true;
		return ;
	}

	readSocketChunk(socketFd, buffer, SEND_SIZE, headerEnd);
	
	int contentLength = extractContentLength(_header);
	if (_headerComplete == true)
	{
		if (contentLength > _currServ.getMaxBody())
		{
			_requestComplete = true;
			return ;
		}
		endRequestWithoutBody(headerEnd);
		storeBodyExceeding(headerEnd);
	}
	if (_body.size() == static_cast<size_t>(contentLength))
	{
		_requestComplete = true;
		return ;
	}
		
}

bool Request::hasTimedOut()
{
	if (_requestComplete == true)
		return (false);
	if (_tracker.isExpired() == true)
	{
		_requestComplete = true;
		_requestSuccess = false;
		_requestTimedOut = true;
		return (true);
	}
	return (false);
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

	if (!(iss >> method >> path >> protocol) || buffer.find("\r") == std::string::npos)
	{
		_requestSuccess = false;
		return ;
	}
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
		protocol[protocol.size() - 1] = 0;
    _requestHeader["Method"] = method;
    _requestHeader["Path"] = path;
    _requestHeader["Protocol"] = protocol; //TODO: check if correct ?? 
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
	setFirstLine(buffer);
	fillHeader(requestStream, buffer);
}

const std::string Request::getCookieValue(const std::string & cookie_name) const
{
	std::string	cookies;
	std::string	cookie_value;
	std::string cookie_name_updated = cookie_name + "=";
	size_t		pos;
	size_t		comma_pos;

	if (cookie_name.length() == 0)
		return ("");
	if (_requestHeader.find("Cookie") == _requestHeader.end())
		return ("");
	cookies = _requestHeader.find("Cookie")->second;
	pos = cookies.find(cookie_name_updated);
	if (pos == std::string::npos)
		return ("");
	pos += cookie_name_updated.size();
	comma_pos = cookies.find(';', pos);
	if (comma_pos == std::string::npos)
	{
		cookie_value = cookies.substr(pos);
	}
	else
	{
		cookie_value = cookies.substr(pos, comma_pos - pos);
	}
	return (cookie_value);
}

unsigned int	Request::getServerIndex() const
{
	return (_serverIndex);
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

const std::string& Request::getHeader() const
{
	return (_header);
}

const bool &Request::isRequestComplete()
{
	return (_requestComplete);
}

const bool &Request::getRequestSuccess() const
{
	return (_requestSuccess);
}

const bool &Request::isRequestTimedOut() const
{
	return (_requestTimedOut);
}

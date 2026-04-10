#include "webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <map>
#include <sstream>

void	Request::clear()
{
	_totalBytes = 0;
	_headerComplete = false;
	_requestCode = OK;
	_requestComplete = false;
	_requestTimedOut = false;
	_discardBody = false;
	_receivedBodyBytes = 0;


	_isChunked = false;
	_encodingCheck = false;
	_chunkDataRead = 0;
	_chunkExpectSize = 0;
	_chunkDone = false;
	_chunkBuffer.clear();


	_requestHeader.clear();
	_header.clear();
	_body.clear();
	_queryString.clear();

	_tracker = TimeoutTracker();
}

Request::Request(const Server &server) : 
_totalBytes(0),_headerComplete(false), _requestCode(OK), _requestComplete(false), _requestTimedOut(false),
_discardBody(false), _isChunked(false), _encodingCheck(false),_chunkDone(false), _chunkExpectSize(0), _chunkDataRead(0),
_receivedBodyBytes(0), _currServ(&server), _tracker(TimeoutTracker())
{
	// std::cerr << "DEBUG: Constructor called for class Request\n" END;
}

Request::~Request()
{
	// std::cerr << "DEBUG: Destructor called for class Request\n";
}

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

static bool checkCrlf(std::string &chunkBuffer, size_t &requestCode, bool &requestComplete)
{
	if (chunkBuffer.size() < 2) // CHECK SIZE BEFORE CHECK CRLF TO AVOID SEGFAULT
		return (false);
	if (chunkBuffer[0] != '\r' || chunkBuffer[1] != '\n')
	{
		requestCode = BAD_REQUEST;
		requestComplete = true;
		return (false);
	}
	return (true);
}

static bool extractSizeLine(std::string &chunkBuffer, size_t &chunkSize){
	size_t pos = chunkBuffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	std::string line = chunkBuffer.substr(0, pos);
	chunkSize = hexToDec(line);
	chunkBuffer.erase(0, pos + 2);
	return (true);
}

static bool handleLastChunk(std::string &chunkBuffer, bool &chunkDone, bool &requestComplete, size_t &requestCode)
{
	if (checkCrlf(chunkBuffer, requestCode, requestComplete) == false)
		return (false);
	chunkBuffer.clear();
	chunkDone = true;
	requestComplete = true;
	return (true);
}

bool Request::_insertInBody()
{
	size_t remainingBytes = _chunkExpectSize - _chunkDataRead;
	size_t availableBytes = _chunkBuffer.size();
	if (availableBytes < remainingBytes + 2)
	{
		size_t actualBytes;
		if (remainingBytes < availableBytes)
			actualBytes = remainingBytes;
		else
			actualBytes = availableBytes;
		if (actualBytes > 0)
		{
			_body.insert(_body.end(), _chunkBuffer.begin(), _chunkBuffer.begin() + actualBytes);
			_chunkDataRead += actualBytes;
			_chunkBuffer.erase(0, actualBytes);
		}
		return (false);
	}
	if (remainingBytes > 0)
	{
		_body.insert(_body.end(), _chunkBuffer.begin(), _chunkBuffer.begin() + remainingBytes);
		_chunkBuffer.erase(0, remainingBytes);
		_chunkDataRead += remainingBytes;
	}
	return (true);
}

void 	Request::_insertChunkedBody(const char *begin, const char *end)
{
	_chunkBuffer.append(begin, end);
	while (1)
	{
		if (_chunkExpectSize == 0 && _chunkDataRead == 0)
			if (extractSizeLine(_chunkBuffer, _chunkExpectSize) == false)
				return ;
		if (_chunkExpectSize == 0){
			handleLastChunk(_chunkBuffer, _chunkDone, _requestComplete, _requestCode);
			return ;
		}
		if (_insertInBody() == false)
			return ;
		if (checkCrlf(_chunkBuffer, _requestCode, _requestComplete) == false)
			return ;
		_chunkBuffer.erase(0, 2);
		_chunkExpectSize = 0;
		_chunkDataRead = 0;
	}
}

void Request::readSocketChunk(int socketFd, char *buffer, size_t bufferSize, size_t &headerEnd)
{
	ssize_t bytesRead = recv(socketFd, buffer, bufferSize, 0);
	if (bytesRead < 0)
	{
		_requestCode = INTERNAL_SERVER_ERROR;
		_requestComplete = true;
		return ;
	}
	if (bytesRead == 0)
	{
		_requestComplete = true;
		return ;
	}
	if (_headerComplete == false)
	{
		_header.append(buffer, bytesRead);
		headerEnd = findHeaderEnd(_header);
	}
	else
	{
		if (_discardBody == false){
			_insertBody(buffer, buffer + bytesRead);
		}
		_receivedBodyBytes += bytesRead;
	}
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
	if (headerEnd == _header.size() && _body.empty())
	{
		_requestComplete = true;
	}
}

void	Request::_insertBody(const char *begin, const char* end){
	if (_isChunked == true)
		_insertChunkedBody(begin, end);
	else
		_body.insert(_body.end(), begin, end);
}

void	Request::storeBodyExceeding(const size_t &headerEnd)
{ 
	if (headerEnd != std::string::npos && headerEnd != _header.size())
	{
		std::string bodyStr = _header.substr(headerEnd);
		if (_discardBody == false)
			_insertBody(bodyStr.c_str(), bodyStr.c_str() + bodyStr.size());
		_receivedBodyBytes += bodyStr.size();
		_header = _header.substr(0, headerEnd);
	}
}

void	Request::_checkEncoding(std::string buffer){
	_encodingCheck = true;
	size_t pos = buffer.find("Transfer-Encoding:");

	if (pos == std::string::npos)
		return ;

	pos += 18;
	while (pos < buffer.size() && (buffer[pos] == ' ' || buffer[pos] == '\t'))
		pos++;

	size_t end = buffer.find("\r", pos);
	if (end == std::string::npos || end == pos)
		return ;
	if (buffer.substr(pos, end - pos) == "chunked")
		_isChunked = true;
}

void Request::receiveRequestChunk(int socketFd)
{	
	char buffer[SEND_SIZE] = {0};
	size_t headerEnd = _header.size();

	readSocketChunk(socketFd, buffer, SEND_SIZE, headerEnd);
	int contentLength = extractContentLength(_header);
	if (_headerComplete == true)
	{
		if (_encodingCheck == false)
			_checkEncoding(_header);
		if (contentLength > 0 && contentLength > _currServ->getMaxBody())
		{
			_requestCode = CONTENT_TOO_LARGE;
			_discardBody = true;
			storeBodyExceeding(headerEnd);
			_body.clear();
			if (_receivedBodyBytes >= static_cast<size_t>(contentLength))
				_requestComplete = true;
			return ;
		}
		if (contentLength <= 0 && _isChunked == false)
			endRequestWithoutBody(headerEnd);
		storeBodyExceeding(headerEnd);
	}
	if (contentLength > 0 && _receivedBodyBytes >= static_cast<size_t>(contentLength))
	{
		_requestComplete = true;
		return ;
	}
}

void	Request::_extractQueryString(std::string &path)
{
	size_t querySeparator = path.find('?');
	if (querySeparator == std::string::npos)
		return ;

	_queryString = path.substr(querySeparator + 1);
	path.erase(querySeparator);
}

void Request::setFirstLine(std::string buffer)
{
    std::istringstream iss(buffer);
    std::string method, path, protocol;

	// std::cout << GREEN BOLD << "BUFFER is --- " << buffer << END << std::endl;
	if (!(iss >> method >> path >> protocol))
	{
		_requestCode = BAD_REQUEST;
		return ;
	}
	else if (buffer.find("\r") == std::string::npos)
	{
		_requestCode = HTTP_VERSION_NOT_SUPPORTED;
		return ;
	}
	std::string rest;
	if (iss >> rest)
	{
		_requestCode = BAD_REQUEST;
		return ;
	}
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
		protocol[protocol.size() - 1] = 0;
	_extractQueryString(path);
	if (path.empty() || path[0] != '/')
	{
		_requestCode = BAD_REQUEST;
		return ;
	}
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
			_requestCode = BAD_REQUEST;
			return ;
		}
		pair.first = buffer.substr(0, sep);
		size_t end = buffer.find('\r');
		if (end == std::string::npos)
		{
			_requestCode = BAD_REQUEST;
			return ;
		}
		sep += 2;
		pair.second = buffer.substr(sep, end - sep);
		_requestHeader.insert(pair);
	}
}

void Request::stateRequestValidity()
{
	if (_requestCode != OK)
		return ;
	//TODO: if POST and no content length its invalid? 
	if (_requestHeader["Protocol"] != "HTTP/1.1")
	{
		_requestCode = HTTP_VERSION_NOT_SUPPORTED;
		return ;
	}
	std::string methods[METHODS_COUNT] = {"HEAD", "GET", "POST", "DELETE"};
	for (int i = 0; i < METHODS_COUNT; i++)
		if (_requestHeader["Method"] == methods[i])
			return ;
	_requestCode = METHOD_NOT_ALLOWED;		
}

static bool isValidHeaderKey(const std::string key){
	for (std::string::const_iterator it = key.begin(); it != key.end(); it++){
		if ((!isalpha(*it) && *it != '-'	))
			return (false);
	}
	return (true);
}

void Request::checkHeader()
{
	for (std::map<std::string, std::string>::iterator it = _requestHeader.begin(); it != _requestHeader.end(); it++){
		if (!isValidHeaderKey(it->first))
			_requestCode = BAD_REQUEST;
	}
}

void Request::parseRequest(std::string request)
{
	std::string buffer;
	std::stringstream requestStream(request, std::ios_base::in);

	std::getline(requestStream, buffer, '\n');
	setFirstLine(buffer);
	fillHeader(requestStream, buffer);
	checkHeader();
	if (_requestCode != OK)
		return ;
	// std::cout << _header << std::endl;
	stateRequestValidity();
}
void				Request::setErrorCode(int errorCode)
{
	_requestCode = errorCode;
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
	{
		// std::cerr << BLUE ITALIC << "Cookie is NOTNOTNOT found :( " << END << std::endl;	
		return ("");
	}
	cookies = _requestHeader.find("Cookie")->second;
	// std::cerr << BLUE ITALIC << "Cookie is found : " << cookies << END << std::endl;
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

// unsigned int	Request::getServerIndex() const
// {
// 	return (_serverIndex);
// }

const Server *	Request::getServer() const
{
	return (_currServ);
}

const std::vector<char>& Request::getBody() const
{
	return (_body);
}

const std::string& Request::getQueryString() const
{
	return (_queryString);
}

const std::map<std::string, std::string>& Request::getMap() const
{
	return (_requestHeader);
}

void	Request::setAsTimeout(int errType)
{
	_requestComplete = true;
	_requestCode = errType;
	_requestTimedOut = true;
}

const std::string& Request::getHeader() const
{
	return (_header);
}

const bool& Request::isRequestComplete()
{
	return (_requestComplete);
}
bool	Request::isKeepAlive() const
{
	if (getMap().find("Connection") != getMap().end())
	{
		if (getMap().find("Connection")->second == "keep-alive")
			return (true);
	}
	return (false);
}

size_t Request::getRequestCode() const
{
	return (_requestCode);
}

const bool& Request::isRequestTimedOut() const
{
	return (_requestTimedOut);
}

#include "webserv.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include <map>
#include <sstream>

void Request::display() const
{
	std::cout << getHeader();
	std::vector<char>::const_iterator it, ite;
	it	= _body.begin();
	ite	= _body.end();
	for (; it != ite; it++)
	{
		std::cout << *it;
	}
	std::cout << std::endl;
}

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

	_tracker = TimeoutTracker();
}

Request::Request(const Server &server, unsigned int serverIndex) : 
_totalBytes(0),_headerComplete(false), _requestCode(OK), _requestComplete(false), _requestTimedOut(false),
_discardBody(false), _isChunked(false), _encodingCheck(false),_chunkDone(false), _chunkExpectSize(0), _chunkDataRead(0),
_receivedBodyBytes(0), _serverIndex(serverIndex), _currServ(&server), _tracker(TimeoutTracker())
{
	std::cerr << "DEBUG: Constructor called for class Request\n" END;
}

Request::~Request()
{
	std::cerr << "DEBUG: Destructor called for class Request\n";
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

void 	Request::_insertChunkedBody(const char *begin, const char *end){
	if (begin == end || _chunkDone == true)
		return ;
	_chunkBuffer.append(begin, end);

	while (1)
	{
		if (_chunkExpectSize == 0 && _chunkDataRead == 0) // IM ON THE SIZE LINE -> EXTRACT THE HEX SIZE CONVERT IT THEN ERASE
		{
			size_t pos = _chunkBuffer.find("\r\n");
			if (pos == std::string::npos)
				return ; // IF CRLF NOT FIND, KEEP READ TO FIND THE END OF THE SIZE LINE
			std::string line = _chunkBuffer.substr(0, pos);
			_chunkExpectSize = hexToDec(line);
			_chunkBuffer.erase(0, pos + 2); // ERASE SIZE + CRLF
		}
		if (_chunkExpectSize == 0)
		{
			if (_chunkBuffer.size() < 2) // CHECK SIZE BEFORE CHECK CRLF TO AVOID SEGFAULT
				return ;
			if (_chunkBuffer[0] != '\r' || _chunkBuffer[1] != '\n')
			{
				_requestCode = BAD_REQUEST;
				_requestComplete = true;
				return ;
			}
			_chunkBuffer.erase(0, 2);
			_chunkDone = true;
			_requestComplete = true;
			for(std::vector<char>::iterator it = _body.begin(); it != _body.end(); it++)
				std::cout << *it;
			std::cout << _body.size();
			return ;
		}
		size_t remainingData = _chunkExpectSize - _chunkDataRead; // GET HOW MANY BYTES ALREADY READ ON PAST LOOP
		size_t availableData = _chunkBuffer.size(); // TOTAL DATA TO READ

		if (availableData < remainingData + 2) // IF SIZE OF THE BUFFER IS LT THE DATA I NEED TO READ -> INSERT THE THE REST OF THE BUFFER / THERE WILL BE MISSING INFO OF THIS CHUNK
		{
			size_t partialData;
			if (remainingData < availableData) // IS REMAINING DATA (LAST SIZE - BYTES READ) LT THE BUFFER WE READ ?  
				partialData = remainingData;
			else
				partialData = availableData;
			if (partialData > 0)
			{
				_body.insert(_body.end(), _chunkBuffer.begin(), _chunkBuffer.begin() + partialData);
				_chunkDataRead += partialData;
				_chunkBuffer.erase(0, partialData);
			}
			return ;
		}

		if (remainingData > 0)
		{
			_body.insert(_body.end(), _chunkBuffer.begin(), _chunkBuffer.begin() + remainingData);
			_chunkBuffer.erase(0, remainingData);
			_chunkDataRead += remainingData;
		}
		// HERE IM SUPPOSED TO HAVE READ THE WHOLE CHUNK, AND AFTER ERASING WHAT I JUST INSERTED IN BUFFER, _CHUNKBUFFER SUPPOSED TO START WITH CRLF. IF NOT, BAD REQUEST
		// IF ITS OK, ERASE CRLF AND KEEP GOING WITH NEXT CHUNK

		if (_chunkBuffer.size() < 2)
			return ;
		if (_chunkBuffer[0] != '\r' || _chunkBuffer[1] != '\n')
		{
			_requestCode = BAD_REQUEST;
			_requestComplete = true;
			return ;
		}
		_chunkBuffer.erase(0, 2); // ERASE LAST \r\n
		_chunkExpectSize = 0; // RESET DATA IF END OF CHUNK
		_chunkDataRead = 0;
	}
}

void Request::readSocketChunk(int socketFd, char *buffer, size_t bufferSize, size_t &headerEnd)
{
	ssize_t bytesRead = recv(socketFd, buffer, bufferSize, 0);
	if (bytesRead < 0)
	{
		_requestCode = BAD_REQUEST;
		_requestComplete = true;
		return ;
	}
	std::cout << BLUE BOLD << std::string(buffer, bytesRead) << END << std::endl;
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
		if (_discardBody == false)
			_insertBody(buffer, buffer + bytesRead);
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
	int contentLength = extractContentLength(_header); // est ce qu on stockerait pas le content length dans request
	
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
			if (_receivedBodyBytes >= static_cast<size_t>(contentLength)) // on peut plus utiliser _body.size() pcq  le body est vide, mais on veut quand meme saovir quand est ce qu on a lu tout le body
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

void Request::displayInfos() const
{
	std::map<std::string, std::string>::const_iterator it = _requestHeader.begin();
	if (it == _requestHeader.end())
		std::cout << "empty request header\n" << std::endl;
	for (; it != _requestHeader.end(); it++)
		std::cout << it->first << " : " << it->second << "\n";
	std::cout << std::endl;
}

void Request::setFirstLine(std::string buffer)
{
    std::istringstream iss(buffer);
    std::string method, path, protocol;

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

void Request::checkHeader(){
	for (std::map<std::string, std::string>::iterator it = _requestHeader.begin(); it != _requestHeader.end(); it++){
		if (!isValidHeaderKey(it->first))
			_requestCode = BAD_REQUEST;
	}
}

void Request::parseRequest(std::string request)
{
	std::string buffer;
	std::stringstream requestStream(request, std::ios_base::in);

	// std::cout << request << "\n\n";
	std::getline(requestStream, buffer, '\n');
	setFirstLine(buffer);
	fillHeader(requestStream, buffer);
	checkHeader();
	if (_requestCode != OK)
		return ;
	stateRequestValidity();
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

const Server *	Request::getServer() const
{
	return (_currServ);
}

const std::vector<char>& Request::getBody() const
{
	return (_body);
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

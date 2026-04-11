#include "webserv.hpp"
// #include "WebServer.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "UploadHandler.hpp"

#include <fstream>
#include <sstream>
#include <dirent.h>

#include <sys/stat.h>
#include <ctime>

const std::string Response::methods[METHODS_COUNT] = {"HEAD", "GET", "POST", "DELETE"};
const std::string Response::mimeTable[MIME_SIZE] = {"html", "css", "png", "ico", "gif", "jpeg", "plain"};

static std::map<int, std::string> initStatusMap() 
{
	std::map<int, std::string> m;

	m[200] = "OK";
	m[201] = "Created";
	m[204] = "No content";
	m[301] = "Move permanently";
	m[302] = "Found";
	m[303] = "See other";
	m[400] = "Bad Request";
	m[403] = "Forbidden";
	m[404] = "Not found";
	m[405] = "Method Not Allowed";
	m[408] = "Request Timeout";
	m[411] = "Length Required";
	m[413] = "Content Too Large";
	m[415] = "Unsupported Media Type";
	m[500] = "Internal Server Error";
	m[504] = "Gateway Timeout";
	m[505] = "HTTP Version Not Supported";
	return (m);
}

const std::map<int, std::string> Response::_statusMap = initStatusMap();

//*********************************************************************************************/
/*										CONSTRUCTORS										  */
//*********************************************************************************************/

void	Response::clear()
{
	_statusCode = 200;
	_bytesSent = 0;

	_directoryPath.clear();

	_rawResponse.clear();
	_header.clear();
	_body.clear();
}

Response::Response(const Server &server) : 
_statusCode(200), _bytesSent(0), _currServ(&server)
{
	// addHeader("Server", );
	// std::cerr << "DEBUG: Constructor called for class Response\n";
}

Response::~Response()
{
	// std::cerr << "DEBUG: Destructor called for class Response\n";
}

void Response::addTimeHeader()
{
	time_t now = time(NULL);
	struct tm gmtTime;
	char date[64];

	_header.erase("Date");
	if (gmtime_r(&now, &gmtTime) == NULL)
	{
		addHeader("Date", "Thu, 01 Jan 1970 00:00:00 GMT");
		return ;
	}
	if (std::strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", &gmtTime) == 0)
	{
		addHeader("Date", "Thu, 01 Jan 1970 00:00:00 GMT");
		return ;
	}
	addHeader("Date", date);
}

//*********************************************************************************************/
/*										METHODS											  	  */
//*********************************************************************************************/

void Response::pickMethod(const Request &_req, Session &session)
{
	if (_statusCode != OK)
		return ;

	static void (Response::*methods[])(const Request &_req, Session &session) = 
	{
		&Response::handleHead,
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};
	for (int i = 0; i < METHODS_COUNT; i++)
		if (_req.getMap().at("Method") == Response::methods[i])
			(this->*methods[i])(_req, session);
}

std::string Response::MIMEtable(std::string localPath)
{
	int		i = 0;
	size_t	mimePos = 0;
	for (; i < MIME_SIZE; i++){
		mimePos = localPath.rfind("." + mimeTable[i]);
		if (mimePos != std::string::npos)
			if ((mimePos + mimeTable[i].size() + 1) == localPath.size())
				break ;
	}
	return ("text/" + mimeTable[i - (i == MIME_SIZE)]);
}


//*********************************************************************************************/
/*										SETTERS											  	  */
//*********************************************************************************************/

void Response::addHeader(std::string key, std::string value)
{
	_header.insert(std::pair<string, string>(key, value));
}
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/

std::map<string, Location>::const_reverse_iterator Response::findBestLocation(std::string &uriPath)
{
	const std::map<std::string, Location>& loc = _currServ->getLocations();
	
	for (std::map<string, Location>::const_reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		const std::string &locationPath = it->first;
		if (uriPath.find(locationPath) == 0
			&& (locationPath == "/"
				|| locationPath.size() == uriPath.size()
				|| (uriPath.size() > locationPath.size() && uriPath[locationPath.size()] == '/')))
		{
			setCurrLocation(it->second);
			uriPath = uriPath.substr(locationPath.size());
			if (uriPath.empty())
				uriPath = "/";
			else if (uriPath[0] != '/')
				uriPath.insert(0, "/");
			return (it);
		}
	}
	return (loc.rend());
}

void Response::validateMethodAllowed(const Location &location, const Request &_req)
{
	std::string method = _req.getMap().at("Method");
	const std::vector<bool>& acceptedMethods = location.getMethods();
	
	int methodIndex = -1;
	for (int i = 0; i < METHODS_COUNT; i++)
	{
		if (methods[i] == method)
		{
			methodIndex = i;
			break;
		}
	}
	
	if (methodIndex == -1 || !acceptedMethods[methodIndex])
	{
		_statusCode = METHOD_NOT_ALLOWED;
	}
}

void Response::appendIndexToDirectory(std::string &localPath, const Location &location)
{
	struct stat s;

	if (stat(localPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode))
	{
		setDirectoryPath(localPath);
		std::string index = location.getIndex();
		if (localPath[localPath.size() - 1] != '/')
			localPath += "/";
		localPath += index;
	}
}

void Response::appendCookieIndexToDirectory(
	std::string &localPath, const std::string &cookie_value)
{
	struct stat s;

	if (stat(localPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode))
	{
		std::string index = cookie_value + ".css";
		if (localPath[localPath.size() - 1] != '/')
			localPath += "/";
		localPath += index;
	}
}

void	Response::buildLocalPathFromCookie(std::string &localPath, const Location & location, const Request &_req)
{
	size_t	pos = localPath.rfind('/');

	if (pos == std::string::npos)
		appendIndexToDirectory(localPath, location);
	pos++;
	
	std::string cookie_name = localPath.substr(pos);
	std::string cookie_value = _req.getCookieValue(cookie_name);

	if (!cookie_value.empty())
		appendCookieIndexToDirectory(localPath, cookie_value);
	else
		appendIndexToDirectory(localPath, location);
}

std::string Response::buildLocalPath(const Request &_req)
{
	std::string uriPath = _req.getMap().at("Path");
	size_t			pos;

	while ((pos = (uriPath.find(".."))) != string::npos)
		uriPath.erase(pos, 2);
	while ((pos = (uriPath.find("//"))) != string::npos)
		uriPath.erase(pos, 1);

	findBestLocation(uriPath);

	if (_currLocation.getRedir().getValid()
		&& uriPath == "/"
		&& _currLocation.getRedir().getRedirPath() != uriPath)
	{
		_statusCode = _currLocation.getRedir().getErrorCode();
		addHeader("Location", _currLocation.getRedir().getRedirPath());
		addHeader("Content-Length", "0");
		return ("");
	}

	validateMethodAllowed(_currLocation, _req);

	std::string localPath = _currLocation.getRoot() + uriPath;
	
	if (_currLocation.isCookie())
		buildLocalPathFromCookie(localPath, _currLocation, _req);
	else
		appendIndexToDirectory(localPath, _currLocation);
	
	return (localPath);
}

//*********************************************************************************************/
/*										UTILS											  	  */
//*********************************************************************************************/

void	Response::insertCookieHeader(std::string cookie)
{
	_header.insert(std::pair<string, string>("Set-Cookie", cookie + ";" + COOKIE_DURATION));
	// std::cout << BLUE ITALIC << cookie << ";" << COOKIE_DURATION << std::endl;
}

void Response::checkFileAccess(std::string &localPath, int requiredAccess)
{
	struct stat s;

	if (stat(localPath.c_str(), &s) == -1) {
		if (!_directoryPath.empty() && _currLocation.isAutoIndex())
			handleAutoIndex();
		else
		{
			_statusCode = NOT_FOUND;
			handleResponsesError();
		}
	}
	else if (!S_ISREG(s.st_mode) || access(localPath.c_str(), requiredAccess) == -1) {
		_statusCode = FORBIDDEN;
		handleResponsesError();
	}
}

bool Response::readFileContent(const std::string &filePath, size_t fileSize)
{
	_body.resize(fileSize);
	std::ifstream infile(filePath.c_str(), std::ios::binary);
	if (fileSize > 0)
		infile.read(&_body[0], fileSize);
	return (true);
}

void Response::parseFormData(std::map<std::string, std::string> &data, const Request &_req, Session &session)
{
	size_t lastPos = 0;
	std::string body(_req.getBody().begin(), _req.getBody().end());

	for (size_t i = body.find("=", 0); i != std::string::npos; i = body.find("=", lastPos))
	{
		size_t endPos = body.find("&", i);
		if (endPos == std::string::npos)
			endPos = body.find("\r\n\r\n");
		std::string key = body.substr((lastPos == 0 ? lastPos : lastPos + 1), i - (lastPos + (lastPos != 0)));
		std::string val = body.substr(i + 1, endPos - (i + 1));

		if (key.find(SESSION_PREFIXE) != std::string::npos)
		{
			key = key.substr(std::string(SESSION_PREFIXE).length());
			session.insertCookieFromPair(std::pair<string, string>(key, val));
		}
		
		data.insert(std::make_pair(key, val));
		lastPos = endPos;
	}
}

void Response::allowedMethodsHeader()
{
	if (_statusCode == METHOD_NOT_ALLOWED)
	{
		const std::vector<bool>&acceptedMethods = _currLocation.getMethods();
		std::string allowedMethods;
		int allowedMethodsCount = 0;
		for (int i = 0; i < METHODS_COUNT; i++)
		{
			if (acceptedMethods[i] == true)
				allowedMethodsCount++;
		}
		int comma = 0;
		for (int i = 0; i < METHODS_COUNT; i++)
		{
			if (acceptedMethods[i] == true)
			{
				allowedMethods += methods[i];
				if (comma++ < allowedMethodsCount - 1)
					allowedMethods += ", ";
			}
		}
		addHeader("Allow", allowedMethods);
		addHeader("Content-Length", "0");	
	}
}

void Response::handleResponsesError()
{
	std::string localPath;
	struct stat s;
	if (_statusCode == METHOD_NOT_ALLOWED){
		allowedMethodsHeader();
		return ;
	}

	mc_is_ite errPage = _currLocation.getErrors().find(_statusCode);
	if (errPage != _currLocation.getErrors().end())
		localPath = errPage->second;
	else
	{
		errPage = _currServ->getServerErrors().find(_statusCode);
		if (errPage != _currServ->getServerErrors().end())
			localPath = errPage->second;
		else
			localPath = _currServ->getServerErrors().find(0)->second;
	}
	if (stat(localPath.c_str(), &s) != -1 && access(localPath.c_str(), R_OK) != -1)
		readFileContent(localPath, s.st_size);
	else
	{
		_body = "<html><head><link rel=\"stylesheet\" href=\"style/\"><title>Error " + intToString(_statusCode) + "</title></head>";
		_body += "<body><h1>Error " + intToString(_statusCode);
		_body += _statusMap.at(_statusCode) + "</h1>";
		_body += "<p>An error occurred while processing your request.</p></body></html>";
	}
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", intToString(_body.length()));
}


void Response::handleAutoIndex()
{
	DIR		*dir = opendir(_directoryPath.c_str());
	dirent	*fileInfo = NULL;

	_body += "<html><head><title>TeamRequette's Webserv - AutoIndex</title></head><body>";
	if (!dir)
	{
		_statusCode = INTERNAL_SERVER_ERROR;
		handleResponsesError();
		return ;
	}
	while ((fileInfo = readdir(dir)) != NULL)
	{
		if (fileInfo->d_name[0] == '.')
			continue ;
		_body += "<a href=\"" + std::string(fileInfo->d_name) + "\">" + std::string(fileInfo->d_name) + "</a><br>\n";
	}
	_body += "</body></html>";
	closedir(dir);

	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", intToString(_body.length()));
}


//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/

void	Response::handleHead(const Request &_req, Session &session)
{
	(void)session;
	(void)_req;
	// std::string localPath;
	struct stat	s;

	// localPath = buildLocalPath(_req);
	checkFileAccess(_localPath, R_OK);
	if ((stat(_localPath.c_str(), &s) == -1))
		return ;
	readFileContent(_localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(_localPath));
	addHeader("Content-Length", intToString(_body.length()));
	_body.clear();
}

void Response::handleGet(const Request &_req, Session &session)
{
	(void)session;
	(void)_req;
	// std::string localPath;
	struct stat s;

	// localPath = buildLocalPath(_req);
	checkFileAccess(_localPath, R_OK);
	if ((stat(_localPath.c_str(), &s) == -1) || _statusCode != OK)
		return ;
	readFileContent(_localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(_localPath));
	addHeader("Content-Length", intToString(_body.length()));
}

bool Response::handleUploadPost(const Request &_req)
{
	struct stat	s;

	checkFileAccess(_localPath, R_OK);
	if ((stat(_localPath.c_str(), &s) == -1) || _statusCode != OK)
		return (false);

	UploadHandler handler(_req, _currLocation.getUploadDir());

	_statusCode = handler.processUpload();
	if (_statusCode != CREATED)
	{
		handleResponsesError();
		return (false);
	}
	return (true);
}

bool Response::handleRegularPost(const Request &_req, Session &session)
{
	struct stat	s;
	std::string localPath = "html/success.html";

	if (stat(localPath.c_str(), &s) == -1)
	{
		handleResponsesError();
		return (false);
	}
	std::map<std::string, std::string> formData;
	parseFormData(formData, _req, session);
	return (true);
}

void Response::handlePost(const Request &_req, Session &session)
{
	std::map<std::string, std::string>::const_iterator contentTypePos = _req.getMap().find("Content-Type");

	if (contentTypePos == _req.getMap().end())
	{

		_statusCode = UNSUPPORTED_MEDIA_TYPE;
		handleResponsesError();
		return ;
	}

	if (UploadHandler::_isMultipart(contentTypePos->second))
	{
		if (!_currLocation.getUpload())
		{
			_statusCode = INTERNAL_SERVER_ERROR;
			handleResponsesError();
			return ;
		}
		if (!handleUploadPost(_req))
			return ;
	}
	else if (!handleRegularPost(_req, session))
		return ;

	_statusCode = SEE_OTHER;
	addHeader("Location", "/success.html");
	addHeader("Content-Length", "0");
}

void Response::handleDelete(const Request &_req, Session &session)
{
	(void)session;
	(void)_req;
	// std::string localPath = buildLocalPath(_req);

    checkFileAccess(_localPath, W_OK);
	
	if (_statusCode == OK)
	{
		if (remove(_localPath.c_str()) != -1)
			_statusCode = NO_CONTENT;
		else
			_statusCode = INTERNAL_SERVER_ERROR;
	}
}

bool Response::isCgi()
{
	size_t suffixPos = _localPath.rfind(".");
	if (suffixPos == std::string::npos)
		return (false);

	std::string suffix = _localPath.substr(suffixPos);
	std::vector<std::string>::const_iterator it = _currLocation.getCgi().begin();
	for (; it != _currLocation.getCgi().end(); it++){
		if (*it == suffix){
			return (true);
		}
	}
	return (false);
}

std::string Response::getChunckResp(int bytes)
{
	std::string chunckResp;

	chunckResp = _rawResponse.substr(_bytesSent, bytes);
	return (chunckResp);
}

void	Response::updateBytesSent(int bytes)
{
	_bytesSent += bytes;
}

bool	Response::isSent()
{
	return (_bytesSent >= _rawResponse.size());
}

void Response::buildRawResponse(const Request &_req)
{
	addTimeHeader();
	addHeader("Server", "webserv/1.0");

	if (_req.getMap().find("Protocol") == _req.getMap().end())
		_rawResponse += "HTTP/1.1 ";
	else
		_rawResponse += _req.getMap().at("Protocol") + " ";
	_rawResponse += intToString(_statusCode) + " " + _statusMap.at(_statusCode) + "\r\n";

	if (_req.isKeepAlive()){
		addHeader("Connection", "keep-alive");
		addHeader("Keep-alive", "timeout=5, max=200");
	}
	std::map<std::string, std::string>::iterator it = _header.begin();
	for (; it != _header.end(); it++)
		_rawResponse += it->first + ": " + it->second + "\r\n";
	
	_rawResponse += "\r\n";

	_headerLength = _rawResponse.size();
	_rawResponse += _body;
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/

void	Response::setDirectoryPath(const std::string &str)
{
	_directoryPath = str;
}

const std::string	Response::getDirectoryPath() const
{
	return (_directoryPath);
}

void Response::setStatusCode(const int statusCode)
{
	_statusCode = statusCode;
}

void Response::appendRawResponse(const char *buffer, size_t bytesRead)
{
	_rawResponse.append(buffer, bytesRead);
}

void Response::setCurrLocation(const Location &loc)
{
	_currLocation = loc;
}
void Response::setLocalPath(const Request &_req)
{
	_localPath = buildLocalPath(_req);
}

const std::string& Response::getRawResponse() const
{
	return (_rawResponse);
}

int Response::getStatusCode() const
{
	return (_statusCode);
}

int Response::getBytesSent() const
{
	return (_bytesSent);
}

int Response::getHeaderLength() const
{
	return (_headerLength);
}

const std::string& Response::getLocalPath() const{
	return (_localPath);
}
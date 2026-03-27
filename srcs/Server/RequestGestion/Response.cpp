#include "webserv.hpp"
// #include "WebServer.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Request.hpp"
#include "UploadHandler.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <sys/wait.h>

const std::string Response::methods[METHODS_COUNT] = {"HEAD", "GET", "POST", "DELETE"};
const std::string Response::mimeTable[MIME_SIZE] = {"html", "css", "png", "ico", "gif", "plain", "jpeg"};

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

	_rawResponse.clear();
	_header.clear();
	_body.clear();
}

Response::Response(const Server &server) : 
_statusCode(200), _bytesSent(0), _currServ(&server)
{
	std::cerr << "DEBUG: Constructor called for class Response\n";
	// TODO : ERROR_CODE dans flux connection
}

Response::~Response()
{
	std::cerr << "DEBUG: Destructor called for class Response\n";
}

//*********************************************************************************************/
/*										METHODS											  	  */
//*********************************************************************************************/

void Response::pickMethod(const Request &_req)
{
	static void (Response::*methods[])(const Request &_req) = 
	{
		&Response::handleHead,
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};
	if (isCgi(_req) == true)
	{
		handleCgi(_req);
		return ;
	}
	else
	{
		for (int i = 0; i < METHODS_COUNT; i++)
			if (_req.getMap().at("Method") == Response::methods[i])
				(this->*methods[i])(_req);
	}
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
	return ("text/" + mimeTable[i]);
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

std::map<string, Location>::const_reverse_iterator Response::findBestLocation(const std::string &uriPath)
{
	const std::map<std::string, Location>& loc = _currServ->getLocations();
	
	for (std::map<string, Location>::const_reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		if (uriPath.find(it->first) == 0)
		{
			setCurrLocation(it->second);
			return (it);
		}
	}
	return (loc.rend());
}

void Response::validateMethodAllowed(const Location &location, const Request &_req)
{
	if (_req.getMap().find("Method") == _req.getMap().end())
			std::cerr << "map::at error 7\n";
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
		std::cout << "Method not accepted\n";
		_statusCode = METHOD_NOT_ALLOWED;
	}
}

//TODO: Envoyer directement la string plutot que la location, pour ne pas faire une fonction speciale cookies
void Response::appendIndexToDirectory(std::string &localPath, const Location &location)
{
	struct stat s;

	if (stat(localPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode))
	{
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
	if (_req.getMap().find("Path") == _req.getMap().end())
			std::cerr << "map::at error 6\n";
	std::string uriPath = _req.getMap().at("Path");
	std::map<string, Location>::const_reverse_iterator bestMatch = findBestLocation(uriPath);
	const Location	&location = bestMatch->second;
	
	validateMethodAllowed(bestMatch->second, _req);
	
	std::string localPath = location.getRoot() + uriPath;

	if (location.isCookie())
		buildLocalPathFromCookie(localPath, location, _req);
	else
		appendIndexToDirectory(localPath, location);
	
	return (localPath);
}

//*********************************************************************************************/
/*										UTILS											  	  */
//*********************************************************************************************/

void	Response::insertCookieHeader(std::string cookie)
{
	_header.insert(std::pair<string, string>("Set-Cookie", cookie + ";" + COOKIE_DURATION));
	std::cout << BLUE ITALIC << cookie << ";" << COOKIE_DURATION << std::endl;
}

void Response::checkFileAccess(std::string &localPath, int requiredAccess)
{
	struct stat s;
	if (stat(localPath.c_str(), &s) == -1) {
		_statusCode = NOT_FOUND;
		handleResponsesError();
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

void Response::parseFormData(std::map<std::string, std::string> &data, const Request &_req)
{
	size_t lastPos = 0;
	std::string body(_req.getBody().begin(), _req.getBody().end());

	for (size_t i = body.find("=", 0); i != std::string::npos; i = body.find("=", lastPos))
	{
		size_t endPos = body.find("&", i);
		if (endPos == std::string::npos)
			endPos = body.find("\r\n\r\n");
		std::string key = body.substr((lastPos == 0 ? lastPos : lastPos + 1), i - (lastPos + (lastPos != 0))); // 🤮🤮🤮
		std::string val = body.substr(i + 1, endPos - (i + 1));

		// if (key.find(SESSION_PREFIXE) != std::string::npos)
		// {
		// 	key = key.substr(std::string(SESSION_PREFIXE).length());
		// 	_session.insertCookieFromPair(std::pair<string, string>(key, val));
		// }
		
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

		for (int i = 0; i < METHODS_COUNT; i++)
		{
			if (acceptedMethods[i] == true)
				allowedMethods += methods[i] + ((i < METHODS_COUNT - 1) ? ", " : "");				
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
		_body = "<html><head><link rel=\"stylesheet\" href=\"css/style/\"><title>Error " + intToString(_statusCode) + "</title></head>";
		_body += "<body><h1>Error " + intToString(_statusCode);
		if (_statusMap.find(_statusCode) == _statusMap.end())
			std::cerr << "map::at error 4\n";
		_body += _statusMap.at(_statusCode) + "</h1>";
		_body += "<p>An error occurred while processing your request.</p></body></html>";
	}
	addHeader("Content-Type", "text/html");
	addHeader("Content-Length", intToString(_body.length()));
}

//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/

void	Response::handleHead(const Request &_req)
{
	std::string localPath;
	struct stat	s;

	localPath = buildLocalPath(_req);
	checkFileAccess(localPath, R_OK);
	if ((stat(localPath.c_str(), &s) == -1))
		return ;
	readFileContent(localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(localPath));
	addHeader("Content-Length", intToString(_body.length()));
	_body.clear();
}

void Response::handleGet(const Request &_req)
{
	std::string localPath;
	struct stat s;

	localPath = buildLocalPath(_req);
	checkFileAccess(localPath, R_OK);
	if ((stat(localPath.c_str(), &s) == -1) || _statusCode != OK)
		return ;
	readFileContent(localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(localPath));
	addHeader("Content-Length", intToString(_body.length()));
}

void Response::handlePost(const Request &_req)
{
	struct stat	s;
	std::map<std::string, std::string>::const_iterator contentTypePos = _req.getMap().find("Content-Type");

	if (contentTypePos == _req.getMap().end())
	{
		_statusCode = UNSUPPORTED_MEDIA_TYPE; // TODO: a confirmer
		handleResponsesError();
		return ;
	}

	if ((_currLocation.getUpload() == true) && UploadHandler::_isMultipart(contentTypePos->second))
	{
		std::string localPath = buildLocalPath(_req);
		checkFileAccess(localPath, R_OK);
		if ((stat(localPath.c_str(), &s) == -1) || _statusCode != OK)
			return ;
		std::string uploadDirectory = _currLocation.getUploadDir();

		UploadHandler handler(_req, uploadDirectory);

		int uploadStatus = handler.processUpload();
		_statusCode = uploadStatus;
		if (uploadStatus != CREATED)
		{
			handleResponsesError();
			return ;
		}
	}
	else
	{
		std::string localPath = "html/success.html"; // SUCCES PAGE IN CONFIG
		if (stat(localPath.c_str(), &s) == -1)
		{
			handleResponsesError();
			return ;
		}
		//TODO: RECUPER DYNAMIQUEMENT SUCCESS HTML via GLOBALSERVER ou REDIR (build full path...)
		// std::string localPath = getSuccessPath();
		std::map<std::string, std::string> formData; // TODO: utiliser cette data
		parseFormData(formData, _req);
	}

	_statusCode = SEE_OTHER;
	addHeader("Location", "/success.html");
	addHeader("Content-Length", "0"); // TODO: 0 ou body size si upload ?
}

void Response::handleDelete(const Request &_req)
{
	std::string localPath = buildLocalPath(_req);

    checkFileAccess(localPath, W_OK);
	
	if (_statusCode == OK)
	{
		if (remove(localPath.c_str()) != -1)
			_statusCode = NO_CONTENT;
		else
			_statusCode = INTERNAL_SERVER_ERROR;
	}
}

void Response::cgiMainProcess(int pipe_in[2], int pipe_out[2], Cgi &cgi, int pid, const Request &_req)
{
	close(pipe_in[0]);
	close(pipe_out[1]);

	if (_req.getMap().find("Method")->second == "POST")
	{
		const std::vector<char>& bodyVec = _req.getBody();
		std::string bodyStr(bodyVec.begin(), bodyVec.end());
		cgi.setBodyMessage(bodyStr); //write in stdout
		if (write(pipe_in[1], cgi.getBodyMessage().c_str(), cgi.getBodyMessage().size()) == -1)
		{
			close(pipe_in[1]);
			close(pipe_out[0]);
			throw ExceptionWriteError();
		}
	}

	close(pipe_in[1]);

	waitpid(pid, NULL, 0);

	char buffer[4096] = {0};
	int bytesRead;

	while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0)
		_rawResponse += buffer;

	close(pipe_out[0]);
	if (bytesRead == -1)
		throw ExceptionReadError(); // vrai error
}

bool Response::isCgi(const Request &_req)
{
	if (_req.getMap().find("Path") == _req.getMap().end())
		std::cerr << "map::at error 3\n";
	std::string uriPath = _req.getMap().at("Path");
	std::map<string, Location>::const_reverse_iterator loc = findBestLocation(uriPath);
	Location location = loc->second;
	size_t suffixPos = uriPath.find(".");
	if (suffixPos == std::string::npos)
		return (false);
	std::string suffix = uriPath.substr(suffixPos);
	std::vector<std::string>::const_iterator it = location.getCgi().begin();
	for (; it != location.getCgi().end(); it++){
		if (*it == suffix)
			return (true);
	}
	return (false);
}

void Response::handleCgi(const Request &_req)
{
	// Cgi cgi(_request, _session);
	Cgi cgi(_req);

	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		throw ExceptionCgiPipeError();
	
	pid_t pid = fork();
	if (!pid)
		cgi.cgiChildProcess(pipe_in, pipe_out);
	else
		cgiMainProcess(pipe_in, pipe_out, cgi, pid, _req);
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
	std::map<std::string, std::string>::iterator it = _header.begin();

	if (_req.getMap().find("Protocol") == _req.getMap().end())
		_rawResponse += "HTTP/1.1 ";
	else
	{
		if (_req.getMap().find("Protocol") == _req.getMap().end())
			std::cerr << "map::at error 1\n";
		_rawResponse += _req.getMap().at("Protocol") + " ";
	}
	if (_statusMap.find(_statusCode) == _statusMap.end())
			std::cerr << "map::at error 2\n";
	_rawResponse += intToString(_statusCode) + " " + _statusMap.at(_statusCode) + "\r\n";

	for (; it != _header.end(); it++)
		_rawResponse += it->first + ": " + it->second + "\r\n";
	
	_rawResponse += "\r\n";

	_headerLength = _rawResponse.size();
	_rawResponse += _body;
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/

void Response::setStatusCode(const int statusCode)
{
	_statusCode = statusCode;
}

void Response::setCurrLocation(const Location &loc)
{
	_currLocation = loc;
}

const std::string& Response::getRawResponse() const
{
	return (_rawResponse);
}

int Response::getBytesSent() const
{
	return (_bytesSent);
}

int Response::getHeaderLength() const
{
	return (_headerLength);
}
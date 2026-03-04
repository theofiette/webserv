#include "webserv.hpp"
#include "WebServer.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Request.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>

const std::string Response::methods[METHODS_COUNT] = {"GET", "POST", "DELETE"};
const std::string Response::mimeTable[MIME_SIZE] = {"html", "css", "png", "ico", "gif", "plain"};

static std::map<int, std::string> initStatusMap() 
{
	std::map<int, std::string> m;

	m[200] = "OK";
	m[201] = "Created";
	m[204] = "No content";
	m[301] = "Move permanently";
	m[302] = "Found";
	m[303] = "See other";
	m[403] = "Forbidden";
	m[404] = "Not found";
	m[405] = "Not allowed";
	m[413] = "Content too large";
	return (m);
}

const std::map<int, std::string> Response::_statusMap = initStatusMap();

//*********************************************************************************************/
/*										CONSTRUCTORS										  */
//*********************************************************************************************/

Response::Response(const Request &request, const Server &server) : _statusCode(200), _currServ(server), _request(request)
{
	_request.displayInfos();
	if (_request.getMap().at("Method") == "POST")
	{
		if (stringToInt(_request.getMap().at("Content-Length")) > _currServ.getMaxBody())
		{
			_statusCode = CONTENT_TOO_LARGE;
			handleGet();
			return ;
		}
	}
	pickMethod();
}

Response::~Response(){}

//*********************************************************************************************/
/*										METHODS											  	  */
//*********************************************************************************************/

std::string Response::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return (ss.str());
}

void Response::pickMethod()
{
	static void (Response::*methods[])() = 
	{
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};
	if (_request.getMap().at("Path").find(".cgi") != std::string::npos) //TODO: aller cher dans la loc
	{
		// handleCgi();
		return ;
	}
	else
	{
		for (int i = 0; i < METHODS_COUNT; i++)
			if (_request.getMap().at("Method") == Response::methods[i])
			{
				try {
					(this->*methods[i])();
					return ;
				}
				catch (ExceptionPathError &e){
					return ;
				}
			}
	}
	throw ExceptionParsingError(); // nouvelle class exception ? 
}

std::string Response::MIMEtable(std::string localPath)
{
	int i = 0;
	for (; i < 5; i++)
		if (localPath.find("." + mimeTable[i]) != std::string::npos)
			break ;
	return ("text/" + mimeTable[i]);
}

//*********************************************************************************************/
/*										SETTERS											  	  */
//*********************************************************************************************/

void Response::setStatusCode()
{
	;
}

void Response::addHeader(std::string key, std::string value)
{
	_header[key] = value;
}

void Response::setErrorResponse(std::string &localPath, std::map<int, std::string> const& errMapLocation, std::string const& root)
{
	// Regarder la gestion du bloc LOCATION
	// Si pas preciser rediriger vers ERROR PAGE par defaut
	mc_is_ite errPage = errMapLocation.find(_statusCode);
	if (errPage != errMapLocation.end())
	{
		localPath = root + "/" + errPage->second;
		_statusCode = MOVED_PERMANENTLY;
	}
	else
	{
		_statusCode = OK;
		errPage = _currServ.getErrorsServer().find(_statusCode);
		if (errPage != _currServ.getErrorsServer().end())
		{
			localPath = errPage->second;
		}
		else{
			localPath = _currServ.getErrorsServer().find(0)->second;
		}
	}
	std::cout << "LOCALPATH : " << localPath << "\n";
}
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/

std::map<string, Location>::const_reverse_iterator Response::findBestLocation(const std::string &uriPath)
{
	const std::map<std::string, Location>& loc = _currServ.getLocations();
	
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

void Response::validateMethodAllowed(const Location &location)
{
	std::string method = _request.getMap().at("Method");
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
		throw ExceptionPathError();
	}
}

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

std::string Response::buildLocalPath()
{
	std::string uriPath = _request.getMap().at("Path");
	std::map<string, Location>::const_reverse_iterator bestMatch = findBestLocation(uriPath);
	
	validateMethodAllowed(bestMatch->second);
	
	std::string localPath = bestMatch->second.getRoot() + uriPath;
	appendIndexToDirectory(localPath, bestMatch->second);
	
	return (localPath);
}

//*********************************************************************************************/
/*										UTILS											  	  */
//*********************************************************************************************/

void Response::checkFileAccess(std::string &localPath, int requiredAccess)
{
	struct stat s;
	if (stat(localPath.c_str(), &s) == -1) {
		_statusCode = NOT_FOUND;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
	}
	else if (!S_ISREG(s.st_mode) || access(localPath.c_str(), requiredAccess) == -1) {
		_statusCode = FORBIDDEN;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
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

void Response::parseFormData(std::map<std::string, std::string> &data)
{
	size_t lastPos = 0;

	for (size_t i = _request.getBody().find("=", 0); i != std::string::npos; i = _request.getBody().find("=", lastPos))
	{
		size_t endPos = _request.getBody().find("&", i);
		if (endPos == std::string::npos)
			endPos = _request.getBody().find("\r\n\r\n");
		std::string key = _request.getBody().substr((lastPos == 0 ? lastPos : lastPos + 1), i - (lastPos + (lastPos != 0))); // 🤮🤮🤮
		std::string val = _request.getBody().substr(i + 1, endPos - (i + 1));
		data.insert(std::make_pair(key, val));
		lastPos = endPos;
	}
}

//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/

void Response::handleGet()
{
	std::string localPath;
	struct stat s;

	if (_statusCode == CONTENT_TOO_LARGE)
	{
		mc_is_ite errPage = _currServ.getErrorsServer().find(_statusCode);
		if (errPage != _currServ.getErrorsServer().end())
			localPath = errPage->second;
		else{
			localPath = _currServ.getErrorsServer().find(0)->second;
		}
	}
	else 
		localPath = buildLocalPath();
	checkFileAccess(localPath, R_OK);
	if ((stat(localPath.c_str(), &s) == -1))
		return ;

	readFileContent(localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(localPath));
	addHeader("Content-Length", intToString(_body.length()));
}

void Response::handlePost()
{
	//TODO: RECUPER DYNAMIQUEMENT SUCCESS HTML via GLOBALSERVER ou REDIR (build full path...)
	struct stat s;
	std::string localPath = "html/success.html";
	// std::string localPath = getSuccessPath();

	if (stat(localPath.c_str(), &s) == -1)
		return ;

	std::map<std::string, std::string> formData; // TODO: utiliser cette data? Du coup on appelera parseform avec &data de la class
	parseFormData(formData);

	_statusCode = SEE_OTHER;
	addHeader("Location", "http://localhost:8080/success.html");
	addHeader("Content-Length", "0");
}

void Response::handleDelete()
{
	std::string localPath = buildLocalPath();
	struct stat s;

	if (stat(localPath.c_str(), &s) == -1)
	{
		_statusCode = NOT_FOUND;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
		return ;
	}

    checkFileAccess(localPath, W_OK);
	
	remove(localPath.c_str());
	_statusCode = NO_CONTENT;
	addHeader("Content-Length", "0");
}

std::string const Response::getResponse(SentResponse &sentresp)
{
	std::string response;

	response += _request.getMap().at("Protocol") + " ";
	response += intToString(_statusCode) + " " + _statusMap.at(_statusCode) + "\r\n";

	for (std::map<std::string, std::string>::iterator it = _header.begin(); it != _header.end(); it++)
		response += it->first + ": " + it->second + "\r\n";
	response += "\r\n";

	sentresp.headerLength = response.size();
	response += _body;
	return (response);
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/

void Response::setCurrLocation(const Location &loc){
	_currLocation = loc;
}

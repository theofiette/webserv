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

Response::Response(Request &request, Server &server) : _statusCode(200), _currServ(server), _request(request)
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
	throw ExceptionParsingError(); // nouvelle class exception ? 
}

std::string Response::MIMEtable(std::string fullPath)
{
	int i = 0;
	for (; i < 5; i++)
		if (fullPath.find("." + mimeTable[i]) != std::string::npos)
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

void Response::setErrorResponse(std::string &fullPath, std::map<int, std::string> const& errMapLocation, std::string const& root)
{
	// Regarder la gestion du bloc LOCATION
	// Si pas preciser rediriger vers ERROR PAGE par defaut
	try{
		fullPath = root + "/" + errMapLocation.at(_statusCode);
		_statusCode = MOVED_PERMANENTLY;
	}
	catch (std::exception &e)
	{
		try{
			fullPath = _currServ.getErrMapServer().at(_statusCode);
			_statusCode = OK;
		}
		catch (std::exception &e){
			fullPath = "html/errors/defaultErrorPage.html";
		}
	}
}
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/

std::string Response::buildFullPath() // passer un int pour la method 
{
    std::string	reqPath = _request.getMap().at("Path");
	std::map<std::string, Location>& loc = _currServ.getLocations();
	std::map<string, Location>::reverse_iterator 	bestMatch = loc.rend();
	std::string fullPath;

	for (std::map<string, Location>::reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		if (reqPath.find(it->first) == 0) //commence bien par la location
		{
			bestMatch = it;
			setCurrLocation(bestMatch->second);
			break ;
		}
	}
	// TODO: check si bloc location / {} est au moins present pour etre sur de matcher sur un truc, pour savoir si on garde ce check
	
	if (bestMatch == loc.rend())
	{
		std::cout << "No matchs found\n";
		throw ExceptionPathError();
	}
	
	const std::vector<bool>& acceptedMethods = bestMatch->second.getMethods();
	if (!acceptedMethods[GET]) // TODO: error 503 -> rediriger vers error page 503 du bloc location OU error 503 general
	{
		std::cout << "Method not accepted\n";
		throw ExceptionPathError();
	}

	fullPath = bestMatch->second.getRoot() + reqPath;
	struct stat s; // TODO: stat doublon dans le handler ensuite
	if (stat(fullPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode)) // Je find l'index par defaut du bloc location ou le global
	{
		std::string index = bestMatch->second.getIndex(); // TODO: si pas d'index, recup le global
		if (fullPath[fullPath.size() - 1] != '/') // TODO: regarder aussi si fullpath commence par / je crois
			fullPath += "/";
		fullPath += index;
	}
	// TODO: check si d'autres choses sont a verifier dans le bloc location
	return (fullPath);
}

//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/

void Response::handleGet()
{
	struct stat s;
	std::string fullPath;

	if (_statusCode == CONTENT_TOO_LARGE)
	{
		try{	
			fullPath = _currServ.getErrMapServer().at(_statusCode);
		}
		catch (std::exception &e){
			fullPath = "html/errors/defaultErrorPage.html";
		}
	}
    fullPath = buildFullPath();
	if (stat(fullPath.c_str(), &s) == -1){
		_statusCode = NOT_FOUND;
		setErrorResponse(fullPath, _currLocation.getErrMap(), _currLocation.getRoot());
	}
	stat(fullPath.c_str(), &s);
    if (!S_ISREG(s.st_mode) || access(fullPath.c_str(), R_OK) == -1) // normalement ici on est pas un dossier
    {
        _statusCode = FORBIDDEN;
        setErrorResponse(fullPath, _currLocation.getErrMap(), _currLocation.getRoot());
		stat(fullPath.c_str(), &s);
    }
    _body.resize(s.st_size);
    std::ifstream infile(fullPath.c_str(), std::ios::binary);
    infile.read(&_body[0], s.st_size);
	addHeader("Content-Type", MIMEtable(fullPath));
	addHeader("Content-Length", intToString(_body.length()));
}

void Response::handlePost(){
	struct stat s;
	std::string fullPath = "html/success.html";
	
	stat(fullPath.c_str(), &s);
	std::map<std::string, std::string> data;
	size_t lastPos = 0;
	for (size_t i = _request.getBody().find("=", 0); i != std::string::npos; i = _request.getBody().find("=", lastPos))
	{
		size_t endPos = _request.getBody().find("&", i);
		if (endPos == std::string::npos)
			endPos = _request.getBody().find("\r\n\r\n");
		std::string key = _request.getBody().substr((lastPos == 0 ? lastPos : lastPos + 1), i - (lastPos + (lastPos != 0)));
		std::string val = _request.getBody().substr(i + 1, endPos - (i + 1));
		std::pair<std::string, std::string> p = std::make_pair(key, val);
		lastPos = endPos;
		data.insert(p);
	}
	// _statusCode = CREATED; //TODO: maybe a use a un moment 
	_statusCode = SEE_OTHER;
	addHeader("Location", "http://localhost:8080/success.html");
	addHeader("Content-Type", MIMEtable(fullPath));
	addHeader("Content-Length", "0");
}

void Response::handleDelete(){
	struct stat s;
	std::string fullPath;

    fullPath = buildFullPath();
	// si on trouve pas -> no content (204)  
	if (stat(fullPath.c_str(), &s) == -1)
	{
		_statusCode = NO_CONTENT;
		return ; // TODO: ERROR PAGE 204
	}
	// stat(fullPath.c_str(), &s);
    if (access(fullPath.c_str(), R_OK) == -1)
    {
        _statusCode = FORBIDDEN;
        setErrorResponse(fullPath, _currLocation.getErrMap(), _currLocation.getRoot());
		stat(fullPath.c_str(), &s);
    }
	remove(fullPath.c_str());
	_body = "<html><h1>File -> " + fullPath + " has been deleted</h1></html>";
	addHeader("Content-Type", MIMEtable(fullPath));
	addHeader("Content-Length", intToString(_body.length()));

}

std::string const Response::getResponse(sentResponse &sentresp)
{
	std::string response;
	response += _request.getMap().at("Protocol") + " ";
	response += intToString(_statusCode) + " " + _statusMap.at(_statusCode) + "\r\n";
	for (std::map<std::string, std::string>::iterator it = _header.begin(); it != _header.end(); it++)
		response += it->first + ": " + it->second + "\r\n";
	response += "\r\n";
	sentresp.headerLength = response.size();
	// if (_request.getMap().at("Method") == "GET")
	response += _body;

	// std::cout << "-----------------------\n";
	// std::cout << response << std::endl;
	// std::cout << "-----------------------\n";

	return (response);
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/



void Response::setCurrLocation(Location &loc){
	_currLocation = loc;
}

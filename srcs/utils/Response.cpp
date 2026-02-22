#include "webserv.hpp"
#include "Response.hpp"
#include "Server.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>

const std::string Response::methods[METHODS_COUNT] = {"GET", "POST", "DELETE"};

//*********************************************************************************************/
/*										CONSTRUCTORS										  */
//*********************************************************************************************/
Response::Response(Request &request, Server &server) : _currServ(server), _request(request)
{
	_statusCode = 200;
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
	int toCall = -1;

	static void (Response::*methods[])() = 
	{
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};

	for (int i = 0; i < METHODS_COUNT; i++)
		if (_request.getMap().at("Method") == Response::methods[i])
			toCall = i;

	switch (toCall)
	{
		case GET:
			(this->*methods[toCall])();
			break;
		case POST:
			(this->*methods[toCall])();
			break;
		case DELETE:
			(this->*methods[toCall])();
			break;
		default:
		{
			// throw ExceptionParsingError(); // nouvelle class exception ? 
		}
	}
}

std::string Response::MIMEtable(std::string fullPath)
{
	if (fullPath.find(".html") != std::string::npos)
		return ("text/html");
	if (fullPath.find(".css") != std::string::npos) // si requete HTML contenant du CSS, y a une deuxieme requete qui suit avec .css
		return ("text/css");
	if (fullPath.find(".png") != std::string::npos)
		return ("image/png");
	if (fullPath.find(".ico") != std::string::npos)
		return ("image/x-icon");
	if (fullPath.find(".gif") != std::string::npos) //pour afficher un gif de con
		return ("image/gif");		
	return ("text/plain"); //default type
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

void Response::setErrorResponse(int statusCode)
{
	// Regarder la gestion du bloc LOCATION
	// Si pas preciser rediriger vers ERROR PAGE par defaut
	(void)statusCode;
}
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/
std::string Response::buildFullPath()
{
    std::string	reqPath = _request.getMap().at("Path");
	std::map<std::string, Location>& loc = _currServ.getLocations();
	mc_sL_ite 	bestMatch = loc.end();
	size_t		maxSize = 0;
	std::string fullPath;

	for (mc_sL_ite it = loc.begin(); it != loc.end(); it++)
	{
		if (reqPath.find(it->first) == 0) //commence bien par la location
		{
			if (it->first.size() > maxSize) // si je trouve un match plus grand 
			{
				bestMatch = it;
				maxSize = it->first.size();
			}
		}
	}
	// TODO: check si bloc location / {} est au moins present pour etre sur de matcher sur un truc, pour savoir si on garde ce check
	
	if (bestMatch == loc.end())
	{
		std::cout << "No matchs found\n";
		throw ExceptionParsingError();
	}
	
	const std::vector<bool>& acceptedMethods = bestMatch->second.getMethods();
	if (!acceptedMethods[GET]) // TODO: on fait quoi dans ce cas la? 
	{
		std::cout << "Method not accepted\n";
		throw ExceptionParsingError();
	}
	// std::string remainingPath = reqPath.substr(bestMatch->first.length());
	// if (remainingPath.empty())
	// 	remainingPath = "/";

	fullPath = bestMatch->second.getRoot() + reqPath;
	
	struct stat s; // TODO: stat doublon dans le handler ensuite
	if (stat(fullPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode)) // Je find l'index par defaut du bloc location ou le global
	{
		std::string index = bestMatch->second.getIndex(); // TODO: si pas d'index, recup le global
		if (fullPath[fullPath.size() - 1] != '/') // TODO: regarder aussi si fullpath commence par / je crois
			fullPath += "/";
		fullPath += index;
	}
	std::cout << "FULL PATH ---------> " << fullPath << std::endl;
	// TODO: check si d'autres choses sont a verifier dans le bloc location
	return (fullPath);
}

//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/
void Response::handleGet()
{
    struct stat s;
    std::string fullPath = buildFullPath();
    
    if (stat(fullPath.c_str(), &s) == -1)
    {
        _statusCode = 404;  // not Found
        throw ExceptionParsingError();
    }
    
    if (!S_ISREG(s.st_mode)) // normalement ici on est pas un dossier
    {
        _statusCode = 403;  // forbidden
        throw ExceptionParsingError();
    }
    
    if (access(fullPath.c_str(), R_OK) == -1)
    {
        _statusCode = 403;  // forbidden
        throw ExceptionParsingError();
    }
    
    std::string body;
    body.resize(s.st_size);
    std::ifstream infile(fullPath.c_str(), std::ios::binary);
    infile.read(&body[0], s.st_size);
	_body = body;

	addHeader("Content-Type", MIMEtable(fullPath));
	addHeader("Content-Length", intToString(_body.length()));
	// std::cout << body << std::endl;
}

void Response::handlePost(){
}

void Response::handleDelete(){
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/
std::string const Response::getResponse()
{
	std::string response;	

	if (_statusCode >= 200 && _statusCode < 300)
	{
		response += _request.getMap().at("Protocol") + " ";
		response += intToString(_statusCode) + " OK\r\n";
	}
	else
	{
		response += _request.getMap().at("Protocol") + " ";
		response += intToString(_statusCode) + " KO\r\n";
	}

	for (std::map<std::string, std::string>::iterator it = _header.begin(); it != _header.end(); it++)
	{
		// std::cout << it->first + ": " + it->second + "\r\n";
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += _body;

	std::cout << "-----------------------\n";
	std::cout << response << std::endl;
	std::cout << "-----------------------\n";

	return (response);
}

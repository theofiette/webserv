#include "webserv.hpp"
#include "Response.hpp"
#include "Server.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>

const std::string Response::methods[METHODS_COUNT] = {"GET", "POST", "DELETE"};
const std::string mimeTable[MIME_SIZE] = {"html", "css", "png", "ico", "gif", "plain"};

//*********************************************************************************************/
/*										CONSTRUCTORS										  */
//*********************************************************************************************/
Response::Response(Request &request, Server &server) : _statusCode(200), _currServ(server), _request(request)
{
	setStatusMap();
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

// void Response::setErrorResponse(std::string &fullPath, Location& loc, int statusCode)
// {
// 	// Regarder la gestion du bloc LOCATION
// 	// Si pas preciser rediriger vers ERROR PAGE par defaut
// 	try{
// 		fullPath = loc.;
// 	}
// 	catch (std::exception &e){
// 		fullPath = _.
// 	}
// 	(void)statusCode;
// }
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/
std::string Response::buildFullPath() // passer un int pour la method 
{
    std::string	reqPath = _request.getMap().at("Path");
	std::map<std::string, Location>& loc = _currServ.getLocations();
	std::map<string, Location>::const_reverse_iterator 	bestMatch = loc.rend();
	std::string fullPath;

	for (std::map<string, Location>::const_reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		if (reqPath.find(it->first) == 0) //commence bien par la location
		{
			bestMatch = it;
			break ;
		}
	}
	std::cout << "LOCATION ROOT IS : " << bestMatch->second.getRoot() << "\n";
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
	// if (stat(fullPath.c_str(), &s) == -1)
    // {
    //     _statusCode = NOT_FOUND;
	// 	setErrorResponse(fullPath, bestMatch, _statusCode);
	// 	// return ;
    // }
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
	std::string fullPath;
	// try{
		fullPath = buildFullPath();
	// }
	// catch (ExceptionPathError &e)
	// {
	// 	return ;
	// };
    // if (stat(fullPath.c_str(), &s) == -1)
    // {
    //     _statusCode = NOT_FOUND;
	// 	setErrorResponse(fullPath, _statusCode);
	// 	// return ;
    // }

    
    if (!S_ISREG(s.st_mode) || access(fullPath.c_str(), R_OK) == -1) // normalement ici on est pas un dossier
    {
        _statusCode = FORBIDDEN;
        throw ExceptionPathError();
    }
	fullPath.resize(s.st_size);
    std::ifstream infile(fullPath.c_str());
    infile.read(&_body[0], s.st_size);

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
		response += intToString(_statusCode) + " " + _statusMap.at(intToString(_statusCode)) + "\r\n";
	}

	for (std::map<std::string, std::string>::iterator it = _header.begin(); it != _header.end(); it++)
	{
		// std::cout << it->first + ": " + it->second + "\r\n";
		response += it->first + ": " + it->second + "\r\n";
	}
	response += "\r\n";
	response += _body;

	std::cout << "-----------------------\n";
	// std::cout << response << std::endl;
	std::cout << "-----------------------\n";

	return (response);
}

void Response::setStatusMap(){
	/*VALID CODES*/
	_statusMap["200"] = "OK";

	_statusMap["201"] = "Created";

	/*REDIR CODES*/
	_statusMap["301"] = "Move permanently";

	
	/*ERROR CODES*/
	_statusMap["403"] = "Forbidden";
	_statusMap["404"] = "Not found";
}
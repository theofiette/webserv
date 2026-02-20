#include "webserv.hpp"
#include "Response.hpp"
#include "Server.hpp"

#include <fstream>

#include <sys/stat.h>

const std::string Response::method[METHODS_COUNT] = {"GET", "POST", "DELETE"};

Response::Response(Request &request, Server &server) : _currServ(server) // TODO: check if we have to check server validity
{
	_statusCode = 404;
	pickMethod(request);
}

Response::~Response(){}

void Response::pickMethod(Request &request)
{
	int toCall = -1;
	static void (Response::*methods[])(Request &request) = 
	{
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};

	for (int i = 0; i < METHODS_COUNT; i++)
		if (request.getMap().at("Method") == Response::method[i])
			toCall = i;
	
	//TODO : Check if method is valid in server config (tfiette) 
	switch (toCall)
	{
		case GET:
			(this->*methods[toCall])(request);
			break;
		case POST:
			(this->*methods[toCall])(request);
			break;
		case DELETE:
			(this->*methods[toCall])(request);
			break;
		default:
		{
			// throw ExceptionParsingError(); // nouvelle class exception ? 
		}
	}
}

void Response::setStatusCode(){
}

void Response::setBody(){}

void Response::addHeader(std::string &key, std::string &value)
{
	(void)key;
	(void)value;
}

void Response::setErrorResponse(int statusCode)
{
	(void)statusCode;
}

// void Response::handleGet(Request &request)
// {
// 	// struct stat s;
// 	std::string path = request.getMap().at("Path");
// 	// Get location with tfiette's map 
// 	std::cout << "Path: " << path << std::endl;

// 	// if (stat(path.c_str(), &s) == -1)
// 	// 	throw ExceptionParsingError(); // to change
// 	// concatenate our path with root of location if match
// 	// try{
// 	// int accessRet = access(path.c_str(), F_OK);
// 	// if (accessRet == 0)
// 	// {
// 		// try{
// 			// accessRet = access(path.c_str(), R_OK);
// 			// if (!accessRet){
// 				std::cout << "IM HERE\n";
// 				std::ifstream infile("fileteocon");
// 				char buf[2000];
// 				infile.rdbuf()->sgetn(buf, 2000);
// 				std::cout << buf << std::endl;
// 			// }
// 		// }
// 		// catch (ExceptionParsingError &e){
// 		// 	;
// 		// }
// 	// }
// 	//Get file, read and fill the body of the response and fill the header
// 	// else if (!accessRet)
// 	// 	throw ExceptionParsingError();
// 	// }
// 	// catch(ExceptionParsingError &e)
// 	// {
// 		// ;// concatenate with correct redirectio
// 	// }
	

// 	// if (s.st_mode == S_IFDIR)
// 	// {
// 	// 		;
// 	// }

// 	// else if (s.st_mode == S_IF){;}
	


// // }
std::string Response::buildFullPath(Request &request)
{
    std::string	reqPath = request.getMap().at("Path");
	std::map<std::string, Location>& loc = _currServ.getLocations();
	mc_sL_ite 	bestMatch = loc.end();
	size_t		maxSize = 0;
	std::string fullPath;

	for (mc_sL_ite it = loc.begin(); it != loc.end(); it++)
	{
		std::cout << "LOCATION BLOC is : " << it->first << "<---" << std::endl;
		std::cout << "LOCATION ROOT is : " << it->second.getRoot() << std::endl;
		std::cout << "REQ PATH  is : " << reqPath << "<---" << std::endl;

		if (reqPath.find(it->first) == 0) //commence bien par la location
		{
			if (it->first.size() > maxSize) // si je trouve un match plus grand 
			{
				bestMatch = it;
				maxSize = it->first.size();
			}
		}
	}
	// TODO: check si bloc location / {} est au moins present pour etre sur de matcher sur un truc
	// pour savoir si on garde ce check
	if (bestMatch == loc.end())
	{
		throw ExceptionParsingError();
	}
	
	const std::vector<bool>& acceptedMethods = bestMatch->second.getMethods();
	if (!acceptedMethods[GET]) 
	{
		throw ExceptionParsingError();
	}

	fullPath = bestMatch->second.getRoot() + reqPath;
	
	struct stat s;
	if (stat(fullPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode))
	{
		std::string index = bestMatch->second.getIndex();
		if (fullPath[fullPath.size() - 1] != '/')
			fullPath += "/";
		fullPath += index;
	}
	
	std::cout << "fullPath: " << fullPath << std::endl;
	// TODO: check si d'autre choses sont a verifier dans le bloc location
	return (fullPath);
}



void Response::handleGet(Request &request)
{
	struct stat s;
	// std::string reqPath = request.getMap().at("Path");

    // std::string path = "fileteocon";
	// (void)request;
	std::string fullPath = buildFullPath(request);

    if (stat(fullPath.c_str(), &s) == -1)
        throw ExceptionParsingError();
    
    std::string body;
    body.resize(s.st_size);
    
    std::ifstream infile(fullPath.c_str(), std::ios::binary);
	
    infile.read(&body[0], s.st_size);

	std::cout << body << std::endl;

    
    // body contient tout le fichier
}

void Response::handlePost(Request &request){
	(void)request;
}

void Response::handleDelete(Request &request){
	(void)request;
}

std::string const Response::getResponse(){
	return ("LECACA");
}
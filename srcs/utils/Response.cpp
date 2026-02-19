#include "webserv.hpp"
#include "Response.hpp"

#include <fstream>


#include <sys/stat.h>

Response::Response(Request &request){
	_statusCode = 404;
	pickMethod(request);
}

Response::~Response(){}


void Response::pickMethod(Request &request)
{
	
	int toCall = -1;
	std::string method[3] = {"GET", "POST", "DELETE"};
	void (Response::*methods[])(Request &request) = {
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};

	for (int i = 0; i < MAX_METHODS; i++)
		if (request.getMap().at("Method") == method[i])
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

void Response::addHeader(std::string &key, std::string &value){
	(void)key;
	(void)value;
}

void Response::setErrorResponse(int statusCode){
	(void)statusCode;
}

void Response::handleGet(Request &request)
{
	struct stat s;
	std::string path = request.getMap().at("Path");
	// Get location with tfiette's map 

	if (stat(path.c_str(), &s) == -1)
		throw ExceptionParsingError(); // to change
	// concatenate our path with root of location if match
	// try{
	// int accessRet = access(path.c_str(), F_OK);
	// if (accessRet == 0)
	// {
		// try{
			// accessRet = access(path.c_str(), R_OK);
			// if (!accessRet){
				std::cout << "IM HERE\n";
				std::ifstream infile("fileteocon");
				char buf[2000];
				infile.rdbuf()->sgetn(buf, 2000);
				std::cout << buf << std::endl;
			// }
		// }
		// catch (ExceptionParsingError &e){
		// 	;
		// }
	// }
	//Get file, read and fill the body of the response and fill the header
	// else if (!accessRet)
	// 	throw ExceptionParsingError();
	}
	// catch(ExceptionParsingError &e)
	// {
		// ;// concatenate with correct redirectio
	// }
	

	// if (s.st_mode == S_IFDIR)
	// {
	// 		;
	// }

	// else if (s.st_mode == S_IF){;}
	


// }

void Response::handlePost(Request &request){
	(void)request;
}

void Response::handleDelete(Request &request){
	(void)request;
}

std::string const Response::getResponse(){
	return ("LECACA");
}
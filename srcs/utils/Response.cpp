#include "webserv.hpp"
#include "Response.hpp"

void (Response::*methods[])() = {&Response::handleGet, &Response::handlePost, &Response::handleDelete};

std::string Response::method[3] = {"GET", "POST", "DELETE"};



Response::Response(Request &request){
	_statusCode = 404;
	pickMethod(request.getMap().at("Method"));
}

Response::~Response(){}


void Response::pickMethod(std::string method)
{
	int toCall = -1;

	for (int i = 0; i < 3; i++) // scale (i < 3) fragile
	{
		if (method == Response::method[i])
			toCall = i;
	}
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
			throw ExceptionParsingError(); // nouvelle class exception ? 
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

void Response::handleGet(){
}

void Response::handlePost(){}

void Response::handleDelete(){}

std::string const Response::getResponse(){
	return ("LECACA");
}
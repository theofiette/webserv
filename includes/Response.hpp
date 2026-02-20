#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"

class Request;
class Server;

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		std::map<std::string, std::string> _statusMap;
		Server &_currServ; // stock &request aussi ?

		static const std::string method[METHODS_COUNT];

		std::string buildFullPath(Request &request);

	public:
		Response(Request &, Server &);
		~Response();
		void setStatusCode();
		void setBody();
		void addHeader(std::string &, std::string &);

		void setErrorResponse(int);

		void pickMethod(Request &);

		void handleGet(Request &);
		void handlePost(Request &);
		void handleDelete(Request &);

		std::string const getResponse();
};
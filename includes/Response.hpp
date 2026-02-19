#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"

class Request;

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		std::map<std::string, std::string> _statusMap;

		std::string method[MAX_METHODS]; // TODO: pas tres scalable si on a dautres methodes a check, vector? 

	public:
		Response(Request &);
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
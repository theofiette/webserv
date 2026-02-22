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

		Server &_currServ;
		Request &_request;

		static const std::string methods[METHODS_COUNT];

		std::string intToString(int);
		std::string MIMEtable(std::string);
		std::string buildFullPath();

	public:
		Response(Request &, Server &);
		~Response();

		void setStatusCode();
		void addHeader(std::string, std::string);
		void setErrorResponse(int);

		void pickMethod();

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse();
};
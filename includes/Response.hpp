#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"

class Request;
class Server;
class Location;

enum ErrorCode{
	FORBIDDEN = 403,
	NOT_FOUND = 404
};

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
		void setStatusMap();
		void addHeader(std::string, std::string);
		void setErrorResponse(std::string&, std::map<std::string, Location>::reverse_iterator,int);

		void pickMethod();

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse();
};
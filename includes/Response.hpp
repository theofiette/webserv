#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"
#include "Location.hpp"

class Request;
class Server;
class Location; // Add this forward declaration before the Response class

enum ErrorCode{
	OK = 200,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	FORBIDDEN = 403,
	NOT_FOUND = 404
};

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		std::map<int, std::string> _statusMap;
		
		Location _currLocation;
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
		void setCurrLocation(Location &l);
		void setStatusMap();
		void addHeader(std::string, std::string);
		void setErrorResponse(std::string&, std::map<int, std::string> const&, std::string const&);

		void pickMethod();

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse();
};
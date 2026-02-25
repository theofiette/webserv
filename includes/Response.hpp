#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"
#include "Location.hpp"


class Request;
// class Response;
class Server;
class Location;

enum ErrorCode{
	OK = 200,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	NOT_ALLOWED = 405,
	CONTENT_TOO_LARGE = 413
};

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		
		Location _currLocation;
		Server &_currServ;
		Request &_request;
		
		static const std::map<int, std::string> _statusMap;
		static const std::string methods[METHODS_COUNT];
		static const std::string mimeTable[MIME_SIZE];

		std::string intToString(int);
		std::string MIMEtable(std::string);
		std::string buildFullPath();

	public:
		Response(Request &, Server &);
		~Response();

		void setStatusCode();
		void setCurrLocation(Location &l);
		void setErrorResponse(std::string&, std::map<int, std::string> const&, std::string const&);
		void addHeader(std::string, std::string);

		void pickMethod();

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse();
};
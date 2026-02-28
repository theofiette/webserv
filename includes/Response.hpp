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
struct SentResponse;

enum ErrorCode{
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
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
		const Server &_currServ;
		const Request &_request;
		
		static const std::map<int, std::string> _statusMap;
		static const std::string methods[METHODS_COUNT];
		static const std::string mimeTable[MIME_SIZE];

		std::string intToString(int);
		std::string MIMEtable(std::string);
		std::string buildFullPath();

		bool checkFileAccess(std::string &fullPath, int requiredAccess);
		bool readFileContent(const std::string &filePath, size_t fileSize);
		void addCommonHeaders(const std::string &filePath);
		void parseFormData(std::map<std::string, std::string> &data);

	public:
		Response(const Request &, const Server &);
		~Response();

		void setStatusCode();
		void setCurrLocation(const Location &);
		void setErrorResponse(std::string&, std::map<int, std::string> const&, std::string const&);
		void addHeader(std::string, std::string);

		void pickMethod();

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse(SentResponse&);
};
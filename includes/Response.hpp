#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"
#include "Location.hpp"
#include "safemath.hpp"
#include "Session.hpp"
#include "Cgi.hpp"


typedef std::multimap<std::string, std::string>::iterator _header_it;

class Request;
class Server;
class Location;
class Cgi;

typedef std::multimap<std::string, std::string> multimap;
typedef std::map<string, Location>::const_reverse_iterator mc_sL_rite;

struct SentResponse;

enum ErrorCode{
	OK = 200,
	CREATED = 201,
	NO_CONTENT = 204,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
	BAD_REQUEST = 400,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	REQUEST_TIMEOUT = 408,
	LENGTH_REQUIRED = 411,
	CONTENT_TOO_LARGE = 413,
	UNSUPPORTED_MEDIA_TYPE = 415,
	INTERNAL_SERVER_ERROR = 500,
	GATEWAY_TIMEOUT = 504,
	HTTP_VERSION_NOT_SUPPORTED = 505
};

typedef struct SentResponse 
{
	std::string rawResponse;
	int			bytesSent;
	int			headerLength;
}	SentResponse;

class Response
{
	private:
		int 				_statusCode;
		multimap			_header;
		std::string 		_body;
		std::string			_localPath;

		/* If empty, the GET request wasnt a directory*/
		std::string			_directoryPath; 

		std::string			_rawResponse;
		size_t				_bytesSent;
		int					_headerLength;
		
		Location 			_currLocation;

		const Server 		*_currServ;

		static const 		std::map<int, std::string> _statusMap;
		static const 		std::string methods[METHODS_COUNT];
		static const 		std::string mimeTable[MIME_SIZE];

		std::string			MIMEtable(std::string);
		std::string			buildLocalPath(const Request &_req);

		mc_sL_rite			findBestLocation(std::string &);
		void 				validateMethodAllowed(const Location &, const Request &_req);

		void 				appendIndexToDirectory(std::string &, const Location &);
		void				appendCookieIndexToDirectory(std::string &, const std::string &);
		void				buildLocalPathFromCookie(std::string &localPath, const Location & location, const Request &_req);

		void 				checkFileAccess(std::string &, int);
		bool 				readFileContent(const std::string &, size_t);
		void 				parseFormData(std::map<std::string, std::string> &, const Request &_req, Session &session);
		bool				handleUploadPost(const Request &_req);
		bool				handleRegularPost(const Request &_req, Session &session);
		void				allowedMethodsHeader();
		void 				addTimeHeader();

	public:
		Response(const Server &);
		~Response();

		void				setCurrLocation(const Location &);
		void				setStatusCode(const int);
		void				appendRawResponse(const char *, size_t);
		void 				addHeader(std::string, std::string);
		void 				insertCookieHeader(std::string);

		void 				pickMethod(const Request &_req, Session &session);
		
		void				handleHead(const Request &_req, Session &session);
		void			 	handleGet(const Request &_req, Session &session);
		void			 	handlePost(const Request &_req, Session &session);
		void			 	handleDelete(const Request &_req, Session &session);
		void 				handleResponsesError();
		void				handleAutoIndex();

		void 				buildRawResponse(const Request &_req);
		std::string 		getChunckResp(int);
		void				updateBytesSent(int);
		bool				isSent();
		bool 				isCgi();

		void				setDirectoryPath(const std::string &);
		const std::string	getDirectoryPath() const;
		void				setLocalPath(const Request &_req);

		const std::string&	getRawResponse() const;
		int					getStatusCode() const;
		int					getHeaderLength() const;
		int					getBytesSent() const;
		const std::string&	getLocalPath() const;

		void				clear();
};
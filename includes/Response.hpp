#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"
#include "Location.hpp"
#include "Cgi.hpp"


class Request;
class Server;
class Location;

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
	NOT_ALLOWED = 405,
	LENGTH_REQUIRED = 413,
	CONTENT_TOO_LARGE = 413
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
		multimap			 _header;
		std::string 		_body;

		std::string			_rawResponse;
		size_t				_bytesSent;
		int					_headerLength;
		int					_contentLength;
		
		Location 			_currLocation;
		const Server 		&_currServ;
		const Request		&_request;
		
		static const 		std::map<int, std::string> _statusMap;
		static const 		std::string methods[METHODS_COUNT];
		static const 		std::string mimeTable[MIME_SIZE];

		std::string			intToString(int);
		std::string			MIMEtable(std::string);
		std::string			buildLocalPath();

		mc_sL_rite			findBestLocation(const std::string &);
		void 				validateMethodAllowed(const Location &);
		void 				appendIndexToDirectory(std::string &, const Location &);

		void 				insertCookieHeader(std::string);

		void 				checkFileAccess(std::string &, int);
		bool 				readFileContent(const std::string &, size_t);
		void 				parseFormData(std::map<std::string, std::string> &);
		void 				handleResponsesError();

	public:
							Response(const Request &, const Server &);
							~Response();

		void				setStatusCode();
		void				setCurrLocation(const Location &);
		void 				addHeader(std::string, std::string);

		void 				pickMethod();

		void			 	handleGet();
		void			 	handlePost();
		void			 	handleDelete();
		void			 	handleCgi();

		void				cgiMainProcess(int[2], int[2], Cgi&, int);
		void 				buildRawResponse();
		std::string 		getChunckResp(int);
		void				updateBytesSent(int);
		bool				isSent();
		bool 				isCgi();

		const std::string&	getRawResponse() const;
		int					getHeaderLength() const;
		int					getBytesSent() const;
};
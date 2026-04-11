#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

#include "Server.hpp"
#include "safemath.hpp"
#include "TimeoutTracker.hpp"

typedef std::map<std::string, std::string>::const_iterator mc_ss_it;
typedef std::map<std::string, std::string>::iterator _req_map_it;

class Request
{
	private:
		size_t								_totalBytes;
		std::map<std::string, std::string>	_requestHeader;
		std::string 						_header;
		std::vector<char> 					_body;
		bool 								_headerComplete;
		size_t								_requestCode;
		bool								_requestComplete;
		bool								_requestTimedOut;
		bool								_discardBody;

		bool								_isChunked;
		bool								_encodingCheck;
		bool								_chunkDone;
		size_t								_chunkExpectSize;
		size_t								_chunkDataRead;
		std::string							_chunkBuffer;
		
		size_t								_receivedBodyBytes;
		std::string							_queryString;

		const Server	*_currServ;
		TimeoutTracker	_tracker;

		void 	setFirstLine(std::string);
		void	_extractQueryString(std::string&);
		void	readSocketChunk(int, char *, size_t, size_t&);

		size_t	findHeaderEnd(std::string);
		void	endRequestWithoutBody(const size_t&);
		void	storeBodyExceeding(size_t const&);
		bool	finalizeWithoutBody(int);

		void	_insertBody(const char *, const char *);
		bool	_insertInBody();
		void 	_insertChunkedBody(const char *, const char *);

		void 	_checkEncoding(std::string);

	public:
		Request(const Server &);
		~Request();

		void 						stateRequestValidity();
		void 						fillHeader(std::stringstream &, std::string &);
		void						checkHeader();
		void						parseRequest(std::string);
		void						receiveRequestChunk(int);		

		bool						isKeepAlive() const;
		const bool &				isRequestComplete();
		void						setAsTimeout(int);

		void						setErrorCode(int);

		const std::string& 			getHeader() const;
		const std::string&			getQueryString() const;
		const std::vector<char>&	getBody() const;
		const Server				*getServer() const;
		size_t						getRequestCode() const;
		const bool&					isRequestTimedOut() const;
		
		const std::map<std::string, std::string>&	getMap() const;
		const std::string							getCookieValue(const std::string &) const;

		void	clear();
	
};

int extractContentLength(const std::string&);

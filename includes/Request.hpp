#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

#include "Server.hpp"
#include "TimeoutTracker.hpp"

typedef std::map<std::string, std::string>::iterator _req_map_it;

class Request
{
	private:
		size_t								_totalBytes;
		std::map<std::string, std::string>	_requestHeader;
		// std::string 						_rawRequest;
		std::string 						_header;
		std::vector<char> 					_body;
		bool 								_headerComplete;
		size_t								_requestCode;
		bool								_requestComplete;
		bool								_requestTimedOut;
		bool								_discardBody;
		// size_t							_contentLength;
		size_t								_receivedBodyBytes;
		int									_serverIndex;

		const Server	*_currServ;
		TimeoutTracker	_tracker;

		void 	setFirstLine(std::string);
		void	readSocketChunk(int, char *, size_t, size_t&);

		size_t	findHeaderEnd(std::string);
		void	endRequestWithoutBody(const size_t&);
		void	storeBodyExceeding(size_t const&);
		bool	finalizeWithoutBody(int);
		// void	setBody(size_t, int);

	public:
		Request();
		Request(const Server &, unsigned int);
		~Request();

		void 				stateRequestValidity();
		void 				fillHeader(std::stringstream &, std::string &);
		void				checkHeader();
		void				parseRequest(std::string);
		void				receiveRequestChunk(int);		
		// // TEMPORAIRE
		void				display() const;
		void 				displayInfos() const;

		bool				isKeepAlive() const;
		const bool &		isRequestComplete();
		void				setAsTimeout();

		const std::string& 			getHeader() const;
		const std::vector<char>&	getBody() const;
		const Server				*getServer() const;
		size_t						getRequestCode() const;
		const bool&					isRequestTimedOut() const;
		
		const std::map<std::string, std::string>&	getMap() const;
		const std::string							getCookieValue(const std::string &) const;
		unsigned int								getServerIndex() const;

		void	clear();
	
};

int extractContentLength(const std::string&);

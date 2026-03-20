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
		std::string 						_body;
		bool 								_headerComplete;
		bool								_requestSuccess;
		bool								_requestComplete;
		bool								_requestTimedOut;
		int									_serverIndex;

		const Server	&_currServ;
		TimeoutTracker	_tracker;

		void 	setFirstLine(std::string);
		void	readSocketChunk(int, char *, size_t, size_t&);

		size_t	findHeaderEnd(std::string);
		void	endRequestWithoutBody(const size_t&);
		void	storeBodyExceeding(size_t const&);
		bool	finalizeWithoutBody(int);
		// void	setBody(size_t, int);

	public:
		Request(const Server &, unsigned int);
		~Request();

		void 				stateRequestValidity();
		void 				fillHeader(std::stringstream &, std::string &);
		void				parseRequest(std::string);
		void				receiveRequestChunk(int);		
		bool				hasTimedOut();
		// // TEMPORAIRE
		void 				displayInfos() const;
		const bool &		isRequestComplete();
		
		const std::string& 	getHeader() const;
		const std::string&	getBody() const;
		const Server &		getServer() const;
		const bool &		getRequestSuccess() const;
		const bool &		isRequestTimedOut() const;
		
		const std::map<std::string, std::string>&	getMap() const;
		const std::string							getCookieValue(const std::string &) const;
		unsigned int								getServerIndex() const;
	
};

int extractContentLength(const std::string&);

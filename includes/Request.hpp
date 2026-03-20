#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

#include "Server.hpp"

typedef std::map<std::string, std::string>::iterator _req_map_it;

class Request
{
	private:
		size_t								_totalBytes;
		std::map<std::string, std::string>	_requestHeader;
		std::string 						_rawRequest;
		std::string 						_body;
		bool								_requestSuccess;
		bool								_requestComplete;
		int									_serverIndex;

		const Server &_currServ;

		void 	setFirstLine(std::string);
		ssize_t	readSocketChunk(int, char *, size_t);

		bool	findBodyStart(size_t &);
		bool	finalizeWithoutBody(int);
		void	setBody(size_t, int);

	public:
		Request(const Server &, unsigned int);
		~Request();

		void 				stateRequestValidity();
		void 				fillHeader(std::stringstream &, std::string &);
		void				checkHeader();
		void				parseRequest(std::string);
		void				receiveRequestChunk(int);		
		// // TEMPORAIRE
		void 				displayInfos() const;
		const bool &		isRequestComplete();
		
		const std::string& 	getRawRequest() const;
		const std::string&	getBody() const;
		const Server &		getServer() const;
		const bool &		getRequestSuccess() const;
		
		const std::map<std::string, std::string>&	getMap() const;
		const std::string							getCookieValue(const std::string &) const;
		unsigned int								getServerIndex() const;
	
};

std::string extractContentLength(const std::string&);

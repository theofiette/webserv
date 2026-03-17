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
		std::map<std::string, std::string>	_reqMap;
		std::string 						_rawRequest;
		std::string 						_body;
		bool								_requestComplete;
		int									_serverIndex;

		const Server &_currServ;

		void 	setFirstLine(std::string);

	public:
		Request(const Server &, unsigned int);
		~Request();

		int		parseRequest(std::string);
		
		const std::string& getRawRequest() const;
		void recvRequest(int);
		bool &isRequestComplete();
		const std::map<std::string, std::string>&	getMap() const;
		const std::string	getCookieValue(const std::string &) const;
		unsigned int getServerIndex() const;
	
	// TEMPORAIRE
	void displayInfos() const;

	const std::string& getBody() const;
	const Server &getServer() const;
};

std::string extractContentLength(std::string buffer);

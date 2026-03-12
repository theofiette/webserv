#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

#include "Server.hpp"

class Request
{
	private:
		size_t								_totalBytes;
		std::map<std::string, std::string>	_requestHeader;
		std::string 						_rawRequest;
		std::string 						_body;
		bool								_requestSuccess;
		bool								_requestComplete;

		const Server &_currServ;

		void 	setFirstLine(std::string);

	public:
		Request(const Server &);
		~Request();

		bool 		stateRequestValidity();
		void 		fillHeader(std::stringstream &, std::string &);
		void 		extractBody(int, size_t);
		void		parseRequest(std::string);
		void		recvRequest(int);		
		// // TEMPORAIRE
		void 				displayInfos() const;
		const bool &		isRequestComplete();
		
		const std::string& 	getRawRequest() const;
		const std::string&	getBody() const;
		const Server &		getServer() const;
		const bool &		getRequestSuccess() const;
		
		const std::map<std::string, std::string>& getMap() const;
	};

std::string extractContentLength(std::string buffer);
int stringToInt(std::string num);
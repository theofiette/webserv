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
		std::map<std::string, std::string>	_reqMap;
		std::string 						_rawRequest;
		std::string 						_body;
		bool								_requestSuccess;
		bool								_requestComplete;

		const Server &_currServ;

		void 	setFirstLine(std::string);

	public:
		Request(const Server &);
		~Request();

		int			parseRequest(std::string);
		void		recvRequest(int);		
		// // TEMPORAIRE
		void displayInfos() const;
		bool &				isRequestComplete();
		
		const std::string& 	getRawRequest() const;
		const std::string&	getBody() const;
		const Server &		getServer() const;
		bool &				getRequestSuccess();
		
		const std::map<std::string, std::string>& getMap() const;
	};

std::string extractContentLength(std::string buffer);
int stringToInt(std::string num);
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
		bool								_requestComplete;

		Server &_currServ;
	public:
		Request(Server &);
		~Request();

		int	parseRequest(std::string);
		void setFirstLine(std::string);
		
		std::string &getRawRequest();
		void recvRequest(int);
		bool &isRequestComplete();
		std::map<std::string, std::string>& getMap();

		// TEMPORAIRE
		void displayInfos();
};

std::string extractContentLength(std::string buffer);
int stringToInt(std::string num);
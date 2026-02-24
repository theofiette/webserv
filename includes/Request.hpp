#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

class Request
{
	private:
		std::map<std::string, std::string>	_reqMap;
		std::string 						_rawRequest;
		bool								_requestComplete;
	public:
		Request(int &);
		~Request();

		int	parseRequest(std::string);
		void setFirstLine(std::string);
		
		std::string &getRawRequest();
		bool &isRequestComplete();
		std::map<std::string, std::string>& getMap();

		// TEMPORAIRE
		void displayInfos();
};

std::string recvRequest(int socketFd);

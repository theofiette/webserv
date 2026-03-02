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
		bool								_requestComplete;

		const Server& _currServ;
	public:
		Request(Server &);
		~Request();

		int	parseRequest(std::string);
		void setFirstLine(std::string);
		
	const std::string& getRawRequest() const;
	void recvRequest(int);
	bool &isRequestComplete();
	const std::map<std::string, std::string>& getMap() const;

	// TEMPORAIRE
	void displayInfos() const;
	const std::string& getBody() const;
	const Server &getServer() const;
};

std::string extractContentLength(std::string buffer);
int stringToInt(std::string num);
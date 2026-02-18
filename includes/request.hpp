#pragma once

#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>
#include <map>
#include "webserv.hpp"

class Request
{
	private:
		int _requestType;
		std::map<std::string, std::string> _reqMap;
		std::string _responseCode = "200";

	public:

		enum requestType
		{
			INVALID = -1,
			GET,
			DELETE,
			POST
		};

		pid_t _pid;
	// Request();
		Request(int &);
		~Request();

		void setRequestType(int);
		void setTarget(std::string target);
		void setProtocole(std::string protocole);
		void setHostPort(std::string port);
		void setConnectionType(std::string connectionType);
		int	parseRequest(std::string request);
		void setFirstLine(std::string tmp);

		void pickRequestType();
		std::string buildReponse();
		void displayInfos();

		void rGet();
		void rDelete();
		void rPost();

		
};

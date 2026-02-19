#pragma once

#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <string>
#include <map>

enum requestType
{
	GET,
	DELETE,
	POST
};

class Request
{
	private:
		int _requestType;
		std::map<std::string, std::string> _reqMap;

	public:
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

		std::map<std::string, std::string>& getMap();

		void pickRequestType();
		void displayInfos();

		void rGet();
		void rDelete();
		void rPost();

		
};

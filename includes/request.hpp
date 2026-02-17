#pragma once

#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>
#include "webserv.hpp"

class Request
{
	private:
		int _requestType;
		std::string _target;
		std::string _protocole;
		std::string _host;
		std::string _port;
		std::string _connectionType;

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

		void pickRequestType();
		void createReponse();
		void displayInfos();

		void rGet();
		void rDelete();
		void rPost();

};

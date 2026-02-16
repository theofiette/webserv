#pragma once

#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>

class Request
{
	private:
		int _requestType;
		std::string _target;
		std::string _protocole;
	public:
		pid_t _pid;
	// Request();
		Request(int &);
		~Request();

		void setRequestType(int);
		void setTarget(std::string target);
		void setProtocole(std::string protocole);

};

enum{
	INVALID = -1,
	GET,
	DELETE,
	POST
};

#include "../srcs/utils/memory.tpp"
#include "exception.hpp"
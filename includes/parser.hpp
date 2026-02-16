#pragma once

#include <iostream>
#include <sys/socket.h>
#include <cstdlib>
#include <string>

class Request
{
	private:
		std::string _requestType;
		std::string _target;
	public:
		pid_t _pid;
	// Request();
		Request(int &);
		~Request();

};

#include "../srcs/utils/memory.tpp"
#include "exception.hpp"
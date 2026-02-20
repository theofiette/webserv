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
		// int _requestType; //USED ??
		std::map<std::string, std::string> _reqMap;

	public:
		Request(int &);
		~Request();

		int	parseRequest(std::string);
		void setFirstLine(std::string);

		std::map<std::string, std::string>& getMap();

		// TEMPORAIRE
		void displayInfos();
};

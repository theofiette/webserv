#pragma once

#include "WebServer.hpp"
#include "Request.hpp"

class Cgi
{
	private:
		std::map<std::string, std::string> _metaVariables;
		std::string _bodyMessage;
	public:
		Cgi();
		Cgi(const Request &);
		~Cgi();
		
		void setupCgiFrame();
};
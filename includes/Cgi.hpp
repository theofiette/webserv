#pragma once

#include "WebServer.hpp"
#include "Request.hpp"

class Cgi
{
	private:
		std::map<std::string, std::string>	_metaVariables;
		std::string 						_bodyMessage;
		char**								_cgiEnv;
	public:
		Cgi();
		Cgi(const Request &);
		~Cgi();
		
		void setupCgiFrame();
		const std::map<std::string, std::string>& getMetaVars() const;
		char* const* getEnv() const;
		const std::string& getBodyMessage() const;

		void setBodyMessage(const std::string &body);


};
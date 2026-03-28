#pragma once

#include "Session.hpp"

class Request;

class Cgi
{
	private:
		std::map<std::string, std::string>	_metaVariables;
		std::string 						_bodyMessage;
		char**								_cgiEnv;

		void cgiMainProcess(int pipe_in[2], int pipe_out[2], int pid, const Request &request, std::string &rawResponse);
	public:
		// Cgi(const Request &, const Session &);
		Cgi(const Request &);
		~Cgi();
		
		void setupCgiFrame();
		const std::map<std::string, std::string>& getMetaVars() const;
		char* const* getEnv() const;
		const std::string& getBodyMessage() const;

		void setBodyMessage(const std::string &);
		void cgiChildProcess(int[2], int[2]);
		void _executeCgi(const Request &, std::string &);
};
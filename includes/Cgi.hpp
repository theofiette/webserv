#pragma once

#include "Session.hpp"

class Request;
class Response;

class Cgi
{
	private:
		std::map<std::string, std::string>	_metaVariables;
		std::string 						_bodyMessage;
		char**								_cgiEnv;

		void				setupEnv();
		void				setupMetavariables(const Request&);
		void				cgiMainProcess(int pipe_in[2], int pipe_out[2], const Request &request, int &readFd);
	public:
		// Cgi(const Request &, const Session &);
		Cgi(const Request &);
		~Cgi();
		

		void				cgiChildProcess(int[2], int[2]);
		
		void				run(const Request &, pid_t &, int &);
		

		void				setBodyMessage(const std::string &);

		char* const*		getEnv() const;
		const std::string&	getBodyMessage() const;
		const std::map<std::string, std::string>& getMetaVars() const;


		static bool			readFromCGI(pid_t &, int &, Response &);
		static void			killProcess(pid_t &, int &);
};
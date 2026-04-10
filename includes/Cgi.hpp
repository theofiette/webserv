#pragma once

#include "Session.hpp"

class Request;
class Response;

class Cgi
{
	private:
		enum	CgiState 
		{
			CGI_IDLE,
			CGI_WRITING_BODY,
			CGI_READING_OUTPUT
		};

		std::map<std::string, std::string>	_metaVariables;
		char**								_cgiEnv;
		
		CgiState							_state;

		pid_t								_pid;
		int									_readFd;
		int									_writeFd;
		
		std::string 						_bodyMessage;
		size_t								_bodyOffset;
		

		void				_failureCgi(std::string, char **);

		void				_setupEnv();
		void				_setupMetavariables(const Response &, const Request &, const Session &);

		void				_setupChildProcess(int[2], int[2]);
		void				_launchCgi(const Response &, const Request &, const Session &);
	
		void				_setupMainProcess(int[2], int[2], const Request &);
		bool				_writeBodyForScript();
		bool				_readFromCGI(Response &);

	public:
		// Cgi(const Request &, const Session &);
		Cgi();
		~Cgi();


		pid_t				run(const Request &);
		void				launchCgi(const Response &, const Request &, const Session &);
		bool				process(short, const Request &, Response &);

		void				clear();
		void				killProcess();

		int					getPollFd() const;
		short				getWantedEvent() const;
		pid_t				getPid() const;
};
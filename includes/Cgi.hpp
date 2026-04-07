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

		void				_failureCgi(std::string, char **);


		void				_setupEnv();
		void				_setupMetavariables(const Request&, const Session &);
		void				_cgiMainProcess(int[2], int[2], const Request &, int &);
		void				_cgiChildProcess(int[2], int[2]);

	public:
		// Cgi(const Request &, const Session &);
		Cgi();
		~Cgi();


		pid_t				run(const Request &, pid_t &, int &);
		void 				launchCgi(const Request &, const Session &);

		void				setBodyMessage(const std::string &);

		char* const*		getEnv() const;
		const std::string&	getBodyMessage() const;
		const std::map<std::string, std::string>& getMetaVars() const;

		static bool			readFromCGI(pid_t &, int &, Response &);

		static void			killProcess(pid_t &, int &);
};
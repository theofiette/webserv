#include "Cgi.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "wrapper.hpp"

#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

void	Cgi::setupMetavariables(const Request &request, const Session &session)
{
	std::map<std::string, std::string> rm = request.getMap();

	_metaVariables["AUTH_TYPE"] = "";
	_metaVariables["CONTENT_LENGTH"] = (rm.find("Content-Length") != rm.end() ? rm.find("Content-Length")->second : "");
	_metaVariables["CONTENT_TYPE"] = (rm.find("Content-Type") != rm.end() ? rm.find("Content-Type")->second : "");
	_metaVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
	_metaVariables["PATH_INFO"] = "";
	_metaVariables["PATH_TRANSLATED"] = "";
	_metaVariables["QUERY_STRING"] = request.getQueryString();
	_metaVariables["REMOTE_ADDR"] = "";
	_metaVariables["REMOTE_HOST"] = "";
	_metaVariables["REMOTE_IDENT"] = "";
	_metaVariables["REMOTE_USER"] = "";
	_metaVariables["REQUEST_METHOD"] = (rm.find("Method") != rm.end() ? rm.find("Method")->second : "");
	_metaVariables["SCRIPT_NAME"] = (rm.find("Path") != rm.end() ? rm.find("Path")->second.substr(1) : ""); // TODO:substr
	_metaVariables["SERVER_NAME"] = request.getServer()->getServerName();
	_metaVariables["SERVER_PORT"] = request.getServer()->getListen();
	_metaVariables["SERVER_PROTOCOL"] = (rm.find("Protocol") != rm.end() ? rm.find("Protocol")->second : "");
	_metaVariables["SERVER_SOFTWARE"] = "";
	_metaVariables["scheme"] = "";
	_metaVariables["protocol-var-name"] = "";
	_metaVariables["extension-var-name"] = "";

	std::map<string, string>::const_iterator it, ite;
	it	= session.getCookies().begin();
	ite	= session.getCookies().end();
	for (; it != ite; it++)
	{
		_metaVariables[it->first] = it->second;
	}
}

void	Cgi::setupEnv()
{
	_cgiEnv = new char*[_metaVariables.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::iterator it = _metaVariables.begin(); it != _metaVariables.end(); it++)
	{
		std::string metaVar = it->first + "=" + it->second;
		_cgiEnv[i] = strdup(metaVar.c_str());
		i++;
	}
	_cgiEnv[i] = NULL;
}

Cgi::Cgi(const Request &request, const Session &session) //TODO: function INIT METAVARIABLES
{
	setupMetavariables(request, session);
	setupEnv();
}

void Cgi::cgiChildProcess(int pipe_in[2], int pipe_out[2])
{
	safeClose(pipe_in[1]);
	safeClose(pipe_out[0]);

	dup2(pipe_in[0], STDIN_FILENO); // pour lire le contenu du POST
	if (dup2(pipe_out[1], STDOUT_FILENO) == -1)
	{
		safeClose(pipe_in[0]);
		safeClose(pipe_out[1]);
		exit(1);
	}
	std::string file = "./cgi/" + (getMetaVars().find("SCRIPT_NAME")->second); // TODO chdir ?
	
	char **tmp = new char*[2];
	tmp[0] = strdup(file.c_str());
	// tmp[0] = strdup((std::string("cgi_tester")).c_str());
	tmp[1] = NULL;
	execve(tmp[0], tmp, getEnv());
	perror("execve");
	exit(0);
}

void Cgi::cgiMainProcess(int pipe_in[2], int pipe_out[2], const Request &request, int &readFd)
{
	safeClose(pipe_in[0]);
	safeClose(pipe_out[1]);

	if (request.getMap().find("Method") != request.getMap().end() && request.getMap().find("Method")->second == "POST")
	{
		const std::vector<char>& bodyVec = request.getBody();
		std::string bodyStr(bodyVec.begin(), bodyVec.end());
		setBodyMessage(bodyStr);
		if (write(pipe_in[1], getBodyMessage().c_str(), getBodyMessage().size()) == -1)
		{
			safeClose(pipe_in[1]);
			safeClose(pipe_out[0]);
			throw ExceptionWriteError();
		}
	}
	safeClose(pipe_in[1]);
	readFd = pipe_out[0];
}

void Cgi::run(const Request &request, pid_t &pid, int &readFd)
{
	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		throw ExceptionCgiPipeError();

	pid = fork();
	if (pid < 0)
	{
		closePipe(pipe_in);
		closePipe(pipe_out);
		throw ExceptionCgiPipeError();
	}
	if (!pid)
		cgiChildProcess(pipe_in, pipe_out);
	else
		cgiMainProcess(pipe_in, pipe_out, request, readFd);
}

bool Cgi::readFromCGI(pid_t &pid, int &readFd, Response &response)
{
	if (readFd < 0)
		return (true);
	char buffer[4096];

	ssize_t bytesRead = read(readFd, buffer, sizeof(buffer));
	if (bytesRead > 0)
	{
		response.appendRawResponse(buffer, bytesRead);
		return (false);
	}
	if (bytesRead == 0) // EOF reached, we close the readFd -> eof -> POLLHUP 
	{
		safeClose(readFd);
		if (pid > 0)
		{
			waitpid(pid, NULL, 0);
			pid = -1;
		}
		return (true);
	}
	if (bytesRead < 0)
		return (false); // TODO: on peut rester stuck ici non ?
	return (true); 
}

void Cgi::killProcess(pid_t &pid, int &readFd)
{
	safeClose(readFd);
	if (pid > 0)
	{
		kill(pid, SIGKILL); // TODO: if (kill == -1? syscall)
		waitpid(pid, NULL, 0);
		pid = -1;
	}
}

Cgi::~Cgi()
{
	if (_cgiEnv)
	{
		for (int i = 0; _cgiEnv[i] != NULL; i++)
			free(_cgiEnv[i]);
		delete[] _cgiEnv;
	}
}

const std::map<std::string, std::string>& Cgi::getMetaVars() const
{
	return (_metaVariables);
}

char* const* Cgi::getEnv() const
{
	return (_cgiEnv);
}

const std::string& Cgi::getBodyMessage() const
{
	return (_bodyMessage);
}

void Cgi::setBodyMessage(const std::string &body)
{
	_bodyMessage = body;
}

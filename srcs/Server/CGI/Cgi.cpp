#include "Cgi.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "wrapper.hpp"
#include "macros.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

Cgi::Cgi()
: _cgiEnv(NULL), _state(CGI_IDLE), _pid(-1), _readFd(-1), _writeFd(-1), _bodyOffset(0)
{}

Cgi::~Cgi()
{}

void	Cgi::_failureCgi(std::string errCode, char **argv)
{
	if (argv)
	{
		free(argv[0]);
		delete[] argv;
	}
	if (_cgiEnv)
	{
		for (int i = 0; _cgiEnv[i] != NULL; i++)
			free(_cgiEnv[i]);
		delete[] _cgiEnv;
		_cgiEnv = NULL;
	}
	throw ExceptionRuntime(errCode);
}

void	Cgi::_setupMetavariables(const Request &request, const Session &session)
{
	std::map<std::string, std::string> rm = request.getMap();

	_metaVariables.clear();
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
	_metaVariables["SCRIPT_NAME"] = (rm.find("Path") != rm.end() ? rm.find("Path")->second.substr(1) : "");
	_metaVariables["SERVER_NAME"] = request.getServer()->getServerName();
	_metaVariables["SERVER_PORT"] = request.getServer()->getListen();
	_metaVariables["SERVER_PROTOCOL"] = (rm.find("Protocol") != rm.end() ? rm.find("Protocol")->second : "");
	_metaVariables["SERVER_SOFTWARE"] = "";
	_metaVariables["scheme"] = "";
	_metaVariables["protocol-var-name"] = "";
	_metaVariables["extension-var-name"] = "";

	std::map<std::string, std::string>::const_iterator it = session.getCookies().begin();
	std::map<std::string, std::string>::const_iterator ite = session.getCookies().end();
	for (; it != ite; ++it)
		_metaVariables[it->first] = it->second;
}

void	Cgi::_setupEnv()
{
	if (_cgiEnv)
	{
		for (int i = 0; _cgiEnv[i] != NULL; ++i)
			free(_cgiEnv[i]);
		delete[] _cgiEnv;
		_cgiEnv = NULL;
	}

	_cgiEnv = new char*[_metaVariables.size() + 1];
	size_t i = 0;
	for (std::map<std::string, std::string>::iterator it = _metaVariables.begin(); it != _metaVariables.end(); ++it)
	{
		std::string metaVar = it->first + "=" + it->second;
		_cgiEnv[i] = strdup(metaVar.c_str());
		++i;
	}
	_cgiEnv[i] = NULL;
}

void	Cgi::_setupChildProcess(int pipeIn[2], int pipeOut[2])
{
	safeClose(pipeIn[1]);
	safeClose(pipeOut[0]);

	dup2(pipeIn[0], STDIN_FILENO);
	if (dup2(pipeOut[1], STDOUT_FILENO) == -1)
	{
		safeClose(pipeIn[0]);
		safeClose(pipeOut[1]);
		exit(1);
	}
	safeClose(pipeIn[0]);
	safeClose(pipeOut[1]);
}

void	Cgi::_launchCgi(const Request &request, const Session &session)
{
	_setupMetavariables(request, session);
	_setupEnv();

	std::string file = "./" + _metaVariables.find("SCRIPT_NAME")->second;
	if (chdir("./cgi") == -1)
		_failureCgi("Children process malfunction : directory not found.", NULL);

	char **argv = new char*[2];
	argv[0] = strdup(file.c_str());
	argv[1] = NULL;
	execve(argv[0], argv, _cgiEnv);
	_failureCgi("Children process malfunction : execve failure", argv);
}

void	Cgi::_setupMainProcess(int pipeIn[2], int pipeOut[2], const Request &request)
{
	safeClose(pipeIn[0]);
	safeClose(pipeOut[1]);

	_readFd = pipeOut[0];
	_writeFd = pipeIn[1];

	Fcntl(_readFd, F_SETFL, O_NONBLOCK);
	Fcntl(_writeFd, F_SETFL, O_NONBLOCK);

	_bodyOffset = 0;
	_bodyMessage.clear();

	if (request.getMap().find("Method") != request.getMap().end() && request.getMap().find("Method")->second == "POST")
	{
		const std::vector<char> &bodyVec = request.getBody();
		_bodyMessage.assign(bodyVec.begin(), bodyVec.end()); // Je stocke le body plutot que faire des calls a request.getBody()
		_state = CGI_WRITING_BODY;
	}
	else
	{
		_state = CGI_READING_OUTPUT;
		safeClose(_writeFd);
	}
}

pid_t	Cgi::run(const Request &request)
{
	int pipeIn[2];
	int pipeOut[2];

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
		throw ExceptionCgiPipeError();
	_pid = fork();
	if (_pid < 0)
	{
		closePipe(pipeIn);
		closePipe(pipeOut);
		throw ExceptionCgiPipeError();
	}
	if (!_pid)
	{
		_setupChildProcess(pipeIn, pipeOut);
		return (_pid);
	}
	else
		_setupMainProcess(pipeIn, pipeOut, request);
	return (_pid);
}

void	Cgi::launchCgi(const Request &request, const Session &session)
{
	_launchCgi(request, session);
}

bool	Cgi::_writeBodyForScript()
{
	if (_writeFd < 0 || _bodyMessage.empty())
	{
		safeClose(_writeFd);
		_state = CGI_READING_OUTPUT;
		return (true);
	}

	ssize_t bytesWrote = write(_writeFd, _bodyMessage.data() + _bodyOffset,
			_bodyMessage.size() - _bodyOffset);
	if (bytesWrote > 0)
		_bodyOffset += static_cast<size_t>(bytesWrote);
	else if (bytesWrote < 0)
		return (false);
	else if (bytesWrote == 0)
	{
		safeClose(_writeFd);
		throw ExceptionWriteError();
	}

	if (_bodyOffset < _bodyMessage.size())
		return (false);

	safeClose(_writeFd);
	_state = CGI_READING_OUTPUT;
	return (true);
}

bool	Cgi::_readFromCGI(Response &response)
{
	if (_readFd < 0)
		return (true);

	char buffer[READ_SIZE];
	ssize_t bytesRead = read(_readFd, buffer, sizeof(buffer));
	if (bytesRead > 0)
	{
		response.appendRawResponse(buffer, bytesRead);
		return (false);
	}
	if (bytesRead == 0)
	{
		if (_pid > 0)
		{
			int childPid = waitpid(_pid, 0, WNOHANG);
			if (childPid == 0)
				return (false);
		}
		safeClose(_readFd);
		_pid = -1;
		_state = CGI_IDLE;
		return (true);
	}
	if (bytesRead < 0)
		return (false);
	return (false);
}

bool	Cgi::process(short revent, const Request &, Response &response)
{
	if (_state == CGI_IDLE)
		return (true);
	if (_state == CGI_WRITING_BODY)
	{
		if (!(revent & POLLOUT))
			return (false);
		if (!_writeBodyForScript())
			return (false);
	}
	if (_state == CGI_READING_OUTPUT)
	{
		if (!(revent & (POLLIN | POLLHUP)))
			return (false);
		return (_readFromCGI(response));
	}
	return (false);
}

int	Cgi::getPollFd() const
{
	if (_state == CGI_WRITING_BODY)
		return (_writeFd);
	if (_state == CGI_READING_OUTPUT)
		return (_readFd);
	return (-1);
}

short	Cgi::getWantedEvent() const
{
	if (_state == CGI_WRITING_BODY)
		return (POLLOUT);
	return (POLLIN);
}

pid_t	Cgi::getPid() const
{
	return (_pid);
}

void	Cgi::killProcess()
{
	safeClose(_readFd);
	safeClose(_writeFd);
	if (_pid > 0)
	{
		kill(_pid, SIGKILL);
		waitpid(_pid, NULL, 0);
		_pid = -1;
	}
	_state = CGI_IDLE;
	_bodyOffset = 0;
	_bodyMessage.clear();
}

void	Cgi::clear()
{
	killProcess();
}

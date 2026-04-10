#include "Connection.hpp"
#include <sys/wait.h>
#include "wrapper.hpp"

void	Connection::setState(ConnectionState state)
{
	if (state == CS_SENDING)
		_wantedEvent = POLLOUT;
	else if (state == CS_WAIT_CGI)
		_wantedEvent = _cgi.getWantedEvent();
	else
		_wantedEvent = POLLIN;
	_state = state;
	// std::cerr << BLUE << "state is now : " << getStateInterpreted() << "\n" END;
}
 // CGI CASE je surveille le pipe du CGI et pas le socket de la connection
int	Connection::getPollFd() const
{
	if (_state == CS_WAIT_CGI)
		return (_cgi.getPollFd());
	return (_socketFd);
}

short	Connection::getWantedEvent()
{
	if (_state == CS_WAIT_CGI)
		return (_cgi.getWantedEvent());
	return (_wantedEvent);
}

string	Connection::getStateInterpreted() const
{
	switch (_state)
	{
		case (CS_NONE):
			return ("CS_NONE");
		case (CS_BUFFERING):
			return ("CS_BUFFERING");
		case (CS_PARSING):
			return ("CS_PARSING");
		case (CS_WRITING):
			return ("CS_WRITING");
		case (CS_WAIT_CGI):
			return ("CS_WAIT_CGI");
		case (CS_SENDING):
			return ("CS_SENDING");
		case (CS_KEEP):
			return ("CS_KEEP");
		case (CS_CLOSE):
			return ("CS_CLOSE");
		default :
			return ("**default**");
	}
}

ConnectionState	Connection::getState() const
{
	return (_state);
}

int	Connection::getFd() const
{
	return (_socketFd);
}

void	Connection::_clear()
{
	_clearCgi();
	_request.clear();
	_response.clear();
}

void	Connection::_clearCgi()
{
	_cgi.clear();
}

void	Connection::_startCgi()
{
	pid_t cgiReturnCode = _cgi.run(_request);
	if (cgiReturnCode == 0)
		setState(CS_LAUNCH_CGI);
	else if (cgiReturnCode < 0)
	{
		_request.setErrorCode(INTERNAL_SERVER_ERROR);
		setState(CS_WRITING);
	}
	else
		setState(CS_WAIT_CGI);
}

void	Connection::_retrieveSession()
{
	std::string	sessionStr;;

	if (_session == NULL)
	{
		sessionStr = _request.getCookieValue("SessionId");
		_session = SessionManager::accessSession(sessionStr , _serverId);
		if (sessionStr.empty())
			_response.insertCookieHeader("SessionId=" + _session->getId());
	}
}

void	Connection::_processCgi(short revent)
{
	if (!_cgi.process(revent, _request, _response))
		return ;
	if (_response.getRawResponse().empty())
	{
		_request.setErrorCode(INTERNAL_SERVER_ERROR);
		setState(CS_WRITING);
	}
	else
		setState(CS_SENDING);
}

void	Connection::_checkTimeout()
{
	if (_timer.isExpired() == true)
	{
		_request.setAsTimeout(REQUEST_TIMEOUT);
		if (_state == CS_BUFFERING)
			setState(CS_WRITING);
		else if (_state == CS_WAIT_CGI)
		{
			_request.setAsTimeout(GATEWAY_TIMEOUT);
			_clearCgi();
			_response.clear();
			setState(CS_WRITING);
		}
		else
			setState(CS_CLOSE);
		_timer = TimeoutTracker();
	}
}

bool	Connection::_processBuffering()
{
	_request.receiveRequestChunk(_socketFd);
	return (_request.isRequestComplete());
}

void	Connection::_processParsing()
{
	_request.parseRequest(_request.getHeader());
	_response.setStatusCode(_request.getRequestCode());
}

void	Connection::_processWriting()
{
	_retrieveSession();
	_response.setLocalPath(_request);
	if (_request.getRequestCode() == OK)
	{
		if (_response.isCgi())
		{
			_startCgi(); // executes CGI script and sets CS_WAIT_CGI
			return ;
		}
		_response.pickMethod(_request, *_session);
	}
	else
	{
		_response.setStatusCode(_request.getRequestCode());
		_response.handleResponsesError();
	}
	_response.buildRawResponse(_request);
}

void	Connection::_processSending()
{
	std::string	chunckedStr;
	int			bytesSent;

	chunckedStr = _response.getChunckResp(SEND_SIZE);

	bytesSent = send(_socketFd, chunckedStr.c_str(), chunckedStr.size(), 0);
	if (bytesSent <= 0)
	{
		setState(CS_CLOSE);
		return ;
	}
	_response.updateBytesSent(bytesSent);
	if (_response.isSent())
	{
		if (_request.isRequestTimedOut())
		{
			setState(CS_CLOSE);
			return ;
		}
		if (_request.getMap().find("Connection") != _request.getMap().end() && _request.getMap().find("Connection")->second == "keep-alive")
			setState(CS_KEEP);
		else
			setState(CS_CLOSE);
	}
}
void	Connection::process(short revent)
{
	if (revent & (POLLERR | POLLNVAL))
	{
		setState(CS_CLOSE);
		return ;
	}
	if (_state != CS_WAIT_CGI && (revent & POLLHUP)) // POLLHUP trigger when CGI pipe is closed so we dont want to close 
	{
		setState(CS_CLOSE);
		return ;
	}
	_checkTimeout();
	switch (_state)
	{
		case (CS_NONE):
			if (!(revent & POLLIN))
					return ;
			setState(CS_BUFFERING);
			break ;
		case (CS_BUFFERING):
			if (!(revent & POLLIN))
				return ;
			if (_processBuffering() == true)
				setState(CS_PARSING);
			else
				break ;
		case (CS_PARSING):
			_processParsing();
			setState(CS_WRITING);
		case (CS_WRITING):
			_processWriting();
			if (getState() == CS_WRITING)
				setState(CS_SENDING);
			break ;
		case (CS_LAUNCH_CGI):
			_cgi.launchCgi(_response, _request, *_session);
			break ;
		case (CS_WAIT_CGI):
			_processCgi(revent); // reads CGI output and sets to SENDING when CGI is done
			break ;
		case (CS_SENDING):
			if (!(revent & POLLOUT))
				return ;
			_processSending();
			break ;
		case (CS_KEEP):
			_clear();
			setState(CS_NONE);
			break ;
		default :
			setState(CS_CLOSE);
			break;
	}
}

Connection &Connection::operator=(const Connection &other)
{
	if (this == &other)
		return (*this);
	this->_socketFd = other._socketFd;
	this->_wantedEvent = other._wantedEvent;
	this->_state = other._state;
	this->_request = other._request;
	this->_response = other._response;
	this->_timer = other._timer;
	this->_cgi = other._cgi;
	this->_session = other._session;
	// _server is a reference and cannot be re-assigned; it should remain bound

	// std::cerr << "DEBUG: Operator = called for class Connection\n";
	return (*this);
}

//	CONSTRUCTOR & DESTRUCTOR //////////////////////////////////////////////////

// Connection::Connection(int fd) :
// {
// 	_socketFd = fd;
// 	_state = CS_NONE;
// 	std::cerr << "DEBUG: Constructor called for class Connection\n";
// }

Connection::Connection(int fd, const Server &server, unsigned int serverId): 
_socketFd(fd), _wantedEvent(POLLIN), _state(CS_NONE), _request(server), _response(server), _cgi(),
_server(server),  _serverId(serverId), _session(NULL)
{
	// std::cerr << "DEBUG: TMP Constructor called for class Connection\n";
}

// Connection::Connection(const Connection &other)
// {
// 	std::cerr << "DEBUG: Copy constructor called for class Connection\n";
// 	*this = other;
// }

Connection::~Connection()
{
	// std::cerr << "DEBUG: Destructor called for class Connection\n";
	// close(_socketFd);
}

///////////////////////////////////////////////////////////////////////////////
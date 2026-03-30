#include "Connection.hpp"
#include <sys/wait.h>
#include "wrapper.hpp"

void	Connection::setState(ConnectionState state)
{
	if (state == CS_SENDING)
		_wantedEvent = POLLOUT;
	else
		_wantedEvent = POLLIN;
	_state = state;
	// std::cout << BLUE << "state is now : " << getStateInterpreted() << "\n" END;
}

int	Connection::getPollFd() const
{
	if (_state == CS_WAIT_CGI && _cgiReadFd >= 0) // CGI CASE je surveille le pipe de lecture du CGI et pas le socket de la connection
		return (_cgiReadFd);
	return (_socketFd);
}

short	Connection::getWantedEvent()
{
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
	_request.clear();
	_response.clear();
}

void	Connection::_clearCgi()
{
	if (_cgiReadFd >= 0 || _cgiPid > 0)
		Cgi::killProcess(_cgiPid, _cgiReadFd);
}

void	Connection::_startCgi()
{
	Cgi cgi(_request, *_session);

	cgi.run(_request, _cgiPid, _cgiReadFd);
	setState(CS_WAIT_CGI);
}

void	Connection::_processCgi(short revent)
{
	if (!(revent & (POLLIN) || revent & POLLHUP))
		return ;

	if(!Cgi::readFromCGI(_cgiPid, _cgiReadFd, _response))
		return ;
	setState(CS_SENDING);
}

void	Connection::_checkTimeout()
{
	if (_timer.isExpired() == true)
	{
		std::cerr << BLUE << "state is now : " << getStateInterpreted() << "\n" END;
		_request.setAsTimeout();
		if (_state == CS_BUFFERING)
			setState(CS_WRITING);
		else
		{
			if (_state == CS_WAIT_CGI)
			{
				std::cout << RED << "Killing CGI fd-> " << _socketFd << END << "\n";
				_clearCgi();
				// if (_cgiReadFd >= 0 || _cgiPid > 0)
				// 	Cgi::killProcess(_cgiPid, _cgiReadFd);
			}
			setState(CS_CLOSE);
		}
		_timer = TimeoutTracker();
	}
}

void	Connection::process(short revent)
{
	std::string chunckedStr;

	if (revent & POLLERR || revent & POLLNVAL)
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
			if (!(revent & POLLIN)) // TODO: maybe erase ?
					return ;
			setState(CS_BUFFERING);

		case (CS_BUFFERING):
			if (!(revent & POLLIN)) // TODO: move those checks ?
					return ;
			_request.receiveRequestChunk(_socketFd);
			if (_request.isRequestComplete())
				setState(CS_PARSING);
			else
				break ;

		case (CS_PARSING):
			_request.parseRequest(_request.getHeader());
			_response.setStatusCode(_request.getRequestCode());
			setState(CS_WRITING);

		case (CS_WRITING):
			if (_session == NULL)
			{
				if (_request.getMap().find("SessionId") != _request.getMap().end())
					_session = SessionManager::accessSession(_request.getMap().at("Method") , _serverId);
				else
				{
					_session = SessionManager::accessSession("" , _serverId);
					
			}

			if (_request.getRequestCode() == OK)
			{
				if (_response.isCgi(_request))
				{
					_startCgi(); // executes CGI script and sets CS_WAIT_CGI
					break ;
				}
				_response.pickMethod(_request, *_session);
			}
			else
			{
				_response.setStatusCode(_request.getRequestCode());
				_response.handleResponsesError();
			}
			_response.buildRawResponse(_request);
			setState(CS_SENDING);
			break ;
	
		case (CS_WAIT_CGI):
			_processCgi(revent); // reads CGI output and sets to SENDING when CGI is done
			break ;

		case (CS_SENDING):
			if (!(revent & POLLOUT))
				return ;
			chunckedStr = _response.getChunckResp(SEND_SIZE);
			int bytesSent;
			std::cout << GREEN ITALIC << chunckedStr << END << "\n";
			bytesSent = send(_socketFd, chunckedStr.c_str(), chunckedStr.size(), 0);
			if (bytesSent <= 0)
			{
				setState(CS_CLOSE);
				return ;
			}
			_response.updateBytesSent(bytesSent);
			if (_response.isSent())
			{
				if (_request.getMap().find("Connection") != _request.getMap().end() && _request.getMap().find("Connection")->second == "keep-alive")
					setState(CS_KEEP);
				else
					setState(CS_CLOSE);
			}
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

// void	Connection::process(short revent)
// {
// 	switch (_state)
// 	{
// 		// std::cerr << "DEBUG: connexion state : ";
// 		case (CS_NONE):
// 			std::cerr << "fd is " << _socketFd << "\n";
// 			std::cerr << "state is CS_NONE\n";
// 			std::cerr << "event is " << revent << std::endl;
// 			setState(CS_BUFFERING);
// 			break;
// 		default :
// 			std::cerr << "fd is " << _socketFd << "\n";
// 			std::cerr << "state is **default**\n";
// 			std::cerr << "event is " << revent << std::endl;
// 			setState(CS_CLOSE);
// 			break;
// 	}
// }

//	CANONICAL /////////////////////////////////////////////////////////////////

Connection &Connection::operator=(const Connection &other)
{
	if (this == &other)
	{
		std::cerr << "this is other\n";
		return (*this);
	}
	this->_socketFd = other._socketFd;
	this->_wantedEvent = other._wantedEvent;
	this->_state = other._state;
	this->_request = other._request;
	this->_response = other._response;
	this->_timer = other._timer;
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

Connection::Connection(int fd, const Server &server, const unsigned int serverId): 
_socketFd(fd), _wantedEvent(POLLIN), _state(CS_NONE), _request(server), _response(server),
_server(server),  _serverId(serverId), _session(NULL)
{
	std::cerr << "DEBUG: TMP Constructor called for class Connection\n";
}

// Connection::Connection(const Connection &other)
// {
// 	std::cerr << "DEBUG: Copy constructor called for class Connection\n";
// 	*this = other;
// }

Connection::~Connection()
{
	std::cerr << "DEBUG: Destructor called for class Connection\n";
	// close(_socketFd);
}

///////////////////////////////////////////////////////////////////////////////
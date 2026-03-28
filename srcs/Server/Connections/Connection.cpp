#include "Connection.hpp"
#include <sys/wait.h>

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
	if (_state == CS_WAIT_CGI && _cgiReadFd >= 0)
		return (_cgiReadFd);
	return (_fd);
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
	return (_fd);
}

void	Connection::_clear()
{
	_request.clear();
	_response.clear();
}

void	Connection::_cleanupCgi(bool endProcess)
{
	if (_cgiReadFd >= 0 || _cgiPid > 0)
	{
		if (endProcess)
			Cgi::stop(_cgiPid, _cgiReadFd);
		else
		{
			if (_cgiReadFd >= 0)
			{
				close(_cgiReadFd);
				_cgiReadFd = -1;
			}
			if (_cgiPid > 0)
			{
				waitpid(_cgiPid, NULL, WNOHANG);
				_cgiPid = -1;
			}
		}
	}
}

void	Connection::_startCgi()
{
	Cgi cgi(_request);

	_cgiRawResponse.clear();
	cgi.start(_request, _cgiPid, _cgiReadFd);
	setState(CS_WAIT_CGI);
}

void	Connection::_processCgi(short revent)
{
	if (!(revent & (POLLIN)))
		return ;

	bool isDone = Cgi::readAvailable(_cgiReadFd, _cgiRawResponse);
	if (!isDone)
		return ;

	Cgi::finish(_cgiPid);
	_response.setRawResponse(_cgiRawResponse);
	setState(CS_SENDING);
}

void	Connection::_checkTimeout(){
	
	if (_timer.isExpired() == true)
	{
		std::cerr << "TIMEDOUT AS CONNECTION : " << _fd << "\n";
		std::cerr << BLUE << "state is now : " << getStateInterpreted() << "\n" END;
		_request.setAsTimeout();
		if (_state == CS_BUFFERING)
			setState(CS_WRITING);
		else if (_state == CS_WRITING || _state == CS_WAIT_CGI)
		{
			// kill(pid);
			std::cerr << "TODO : TIMEOUT CGI\n";
		}
		else
			setState(CS_CLOSE);
		_timer = TimeoutTracker();
	}
}

void	Connection::processTmp(short revent)
{
	std::cerr << RED << "PROCESS: fd is " << _fd << "\n";
	std::string chunckedStr;

	// std::cerr << "fd is " << _fd << "\n";
	// std::cerr << "event is " << revent << std::endl;
	// std::cerr << "DEBUG: connexion state : ";
	if (revent & POLLERR || revent & POLLHUP || revent & POLLNVAL)
	{
		std::cerr << RED BOLD << "Error !!!" << std::endl;
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
		case (CS_BUFFERING):
			if (!(revent & POLLIN))
					return ;
			_request.receiveRequestChunk(_fd);
			if (_request.isRequestComplete())
				setState(CS_PARSING);
			else
				break ;
		case (CS_PARSING):
			_request.parseRequest(_request.getHeader());
			_response.setStatusCode(_request.getRequestCode());
			setState(CS_WRITING);
		case (CS_WRITING):
			if (_request.getRequestCode() == OK)
			{
				if (_response.isCgi(_request))
				{
					_startCgi();
					break ;
				}
				_response.pickMethod(_request);
			}
			else{
				_response.setStatusCode(_request.getRequestCode());
				_response.handleResponsesError();
			}
			_response.buildRawResponse(_request);
			setState(CS_SENDING);
			break ;
		case (CS_WAIT_CGI):
			_processCgi(revent);
			break ;
		case (CS_SENDING):
			if (!(revent & POLLOUT))
				return ;
			chunckedStr = _response.getChunckResp(SEND_SIZE);
			int bytesSent;
			std::cout << GREEN ITALIC << "SENDING>> " << chunckedStr << END << "\n";
			bytesSent = send(_fd, chunckedStr.c_str(), chunckedStr.size(), 0);
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
// 			std::cerr << "fd is " << _fd << "\n";
// 			std::cerr << "state is CS_NONE\n";
// 			std::cerr << "event is " << revent << std::endl;
// 			setState(CS_BUFFERING);
// 			break;
// 		default :
// 			std::cerr << "fd is " << _fd << "\n";
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
	this->_fd = other._fd;
	this->_wantedEvent = other._wantedEvent;
	this->_state = other._state;
	this->_request = other._request;
	this->_response = other._response;
	this->_timer = other._timer;
	// _server is a reference and cannot be re-assigned; it should remain bound

	// std::cerr << "DEBUG: Operator = called for class Connection\n";
	return (*this);
}

//	CONSTRUCTOR & DESTRUCTOR //////////////////////////////////////////////////

// Connection::Connection(int fd) :
// {
// 	_fd = fd;
// 	_state = CS_NONE;
// 	std::cerr << "DEBUG: Constructor called for class Connection\n";
// }

Connection::Connection(int fd, const Server &server, unsigned int tmp_i): 
_fd(fd), _wantedEvent(POLLIN), _state(CS_NONE), _request(server, tmp_i), _response(server), _server(server)
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
	// close(_fd);
}

///////////////////////////////////////////////////////////////////////////////
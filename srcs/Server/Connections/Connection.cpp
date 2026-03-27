#include "Connection.hpp"

void	Connection::setState(ConnectionState state)
{
	if (state == CS_SENDING)
		_wantedEvent = POLLOUT;
	else
		_wantedEvent = POLLIN;
	_state = state;
	std::cout << BLUE << "state is now : " << getStateInterpreted() << "\n" END;
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
	if (_request.isRequestTimedOut() && (_state < CS_SENDING))
	{
		setState(CS_CLOSE);
		return ;
	}
	switch (_state)
	{
		case (CS_NONE):
			setState(CS_BUFFERING);
			break;
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
				_response.pickMethod(_request);
			else
				_response.handleResponsesError();
			_response.buildRawResponse(_request);
			setState(CS_SENDING);
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

void	Connection::process(short revent)
{
	switch (_state)
	{
		// std::cerr << "DEBUG: connexion state : ";
		case (CS_NONE):
			std::cerr << "fd is " << _fd << "\n";
			std::cerr << "state is CS_NONE\n";
			std::cerr << "event is " << revent << std::endl;
			setState(CS_BUFFERING);
			break;
		default :
			std::cerr << "fd is " << _fd << "\n";
			std::cerr << "state is **default**\n";
			std::cerr << "event is " << revent << std::endl;
			setState(CS_CLOSE);
			break;
	}
}

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
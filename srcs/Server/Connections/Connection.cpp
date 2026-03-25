#include "Connection.hpp"

// short Connection::getEvent() const
// {
// 	return (_event);
// }
void	Connection::setState(ConnectionState state)
{
	_state = state;
}

ConnectionState	Connection::getState() const
{
	return (_state);
}

int	Connection::getFd() const
{
	return (_fd);
}

void	Connection::processTmp(short revent)
{
	std::string chunckedStr;

	std::cerr << "fd is " << _fd << "\n";
	std::cerr << "event is " << revent << std::endl;
	std::cerr << "DEBUG: connexion state : ";
	switch (_state)
	{
		case (CS_NONE):
			std::cerr << "CS_NONE\n";
			setState(CS_PARSING);
			break;
		case (CS_PARSING):
			std::cerr << "CS_PARSING\n";
			if (!(revent & POLLIN))
				return ;
			_request.receiveRequestChunk(_fd);
			if (_request.isRequestComplete())
				setState(CS_PARSED);
			break ;
		case (CS_PARSED):
			std::cerr << "CS_PARSED\n";
			_request.parseRequest(_request.getRawRequest());
			std::cerr << YELLOW << _request.getRawRequest() << END << std::endl;
			setState(CS_WRITING);
			break ;
		case (CS_WRITING):
			std::cerr << "CS_WRITING\n";
			//session here
			if (_response.checkRequestValidity(_request) == true)
				_response.pickMethod(_request);
			else
				_response.handleResponsesError();
			_response.buildRawResponse(_request);
			setState(CS_SENDING);
			break ;
		case (CS_SENDING):
			std::cerr << "CS_SENDING\n";
			if (!(revent & POLLOUT))
				return ;
			chunckedStr = _response.getChunckResp(SEND_SIZE);
			int bytesSent;
			std::cout << GREEN ITALIC << chunckedStr << END << "\n";
			bytesSent = send(_fd, chunckedStr.c_str(), chunckedStr.size(), 0);
			if (bytesSent < 0)
			{
				setState(CS_CLOSE);
				return ;
			}
			_response.updateBytesSent(bytesSent);
			if (_response.isSent())
				setState(CS_CLOSE);
			break ;
		default :
			std::cerr << "**default**\n";
			setState(CS_CLOSE);
			break;
	}
}

void	Connection::process(short revent)
{
	switch (_state)
	{
		std::cerr << "DEBUG: connexion state : ";
		case (CS_NONE):
			std::cerr << "fd is " << _fd << "\n";
			std::cerr << "state is CS_NONE\n";
			std::cerr << "event is " << revent << std::endl;
			setState(CS_PARSING);
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
	this->_state = other._state;

	std::cerr << "DEBUG: Operator = called for class Connection\n";
	return (*this);
}

//	CONSTRUCTOR & DESTRUCTOR //////////////////////////////////////////////////

// Connection::Connection(int fd) :
// {
// 	_fd = fd;
// 	_state = CS_NONE;
// 	std::cerr << "DEBUG: Constructor called for class Connection\n";
// }

Connection::Connection(int fd, const Server &tmp_serv, unsigned int tmp_i) : _request(tmp_serv, tmp_i), _response(tmp_serv)
{
	_fd = fd;
	_state = CS_NONE;
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
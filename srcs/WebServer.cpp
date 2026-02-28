
#include "WebServer.hpp"

# include "errno.h"

void	WebServer::_initMasterSocket()
{
	struct addrinfo *gai;
	struct addrinfo hints;
	int				tmp = 1;

	t_bzero< struct addrinfo >(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
	_masterSocket = socket(AF_INET, SOCK_STREAM, 0); // TODO check protocol
	//TODO gai_error
	if (
		_masterSocket == -1														||
		fcntl(_masterSocket, F_SETFL, O_NONBLOCK) == - 1 						||
		getaddrinfo(NULL, LOCALHOST, &hints, &gai) == -1 						||
		setsockopt(_masterSocket, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int))	||
		bind(_masterSocket, gai->ai_addr, gai->ai_addrlen) == -1				||
		listen(_masterSocket, MAX_CLIENTS) == -1)
		{
			std::cerr << strerror(errno) << std::endl;
			throw( ExceptionBadInit() );
		}
	freeaddrinfo(gai);
}

void	WebServer::_initPollFds()
{
	t_bzero(_pollfds, sizeof(_pollfds));
	_pollfds[0].fd = _masterSocket;
	_pollfds[0].events = POLLIN;
	_size++; // (set size at 1);
	_tempSize++;
}

void	WebServer::_processConnections()
{
	int	socketFd;

	std::cout << "IN REQUEST CONNECTION \n\n";
	while(1)
	{
		socketFd = accept(_masterSocket, 0, 0); // TODO check args
		if (socketFd < 0)
		{
			// TODO error here ?
			break ;
		}
		_pollfds[_size].fd = socketFd; 
		_pollfds[_size].events = POLLIN;
		_size++;
	}
}

void	WebServer::_loopUpdate()
{
	if (_size != _tempSize) // not finished + don't like
	{
		for (int i = 0; i < _size; i++)
		{
			if (_pollfds[i].fd == -1)
			{
				for (int j = i; j < _size - 1; j++)
				{
					_pollfds[j] = _pollfds[j + 1];
				}
				_tempSize--;
				i--;
			}
		}
	}
	_size = _tempSize;
}

void	WebServer::loop()
{
	pollfd currRequest;

	while (1) // TODO deal with error
	{
		_awaitRequests();
		std::cout << "OUT OF POLL\n";
		for (int i = 0; i < _size; i++)
		{
			currRequest = _pollfds[i];
			// if (!currRequest.revents)
			// 	continue ;
			// if (currRequest.revents != POLLIN)
			// {
			// 	// TODO error here ?
			// 	break;
			// }
			if (currRequest.fd == _masterSocket)
				_processConnections();
			else
				_processRequest(currRequest);
		}
		_loopUpdate();
	}
}

void	WebServer::display()
{
	std::vector<Server>::iterator it, ite;
	it	= _config.servers.begin();
	ite	= _config.servers.end();
	for (; it != ite; it++)
		it->display();
}

void	WebServer::_processRequest(pollfd &request)
{
	(void)request;
}

void	WebServer::_awaitRequests()
{
	std::cout << "AWAIT\n";
	std::cout << _tempSize << "\n";
	std::cout << _size << "\n";
	poll(_pollfds, _size, TIMEOUT);
}

// CONSTRUCTORS

WebServer::WebServer(const WebServerConfig &config)
{
	_config = config;
}

// DESTRUCTOR

WebServer::~WebServer()
{
	close(_masterSocket);
	// std::cout << "Destructor called for class WebServer" << std::endl;
}

// WEBSERVERCONFIG

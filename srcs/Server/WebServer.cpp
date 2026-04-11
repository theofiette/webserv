#include <csignal>

#include "WebServer.hpp"
#include "wrapper.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "safemath.hpp"

# include "errno.h"

void	WebServer::_initReferenceSocket(struct addrinfo &hints)
{
	t_bzero<struct addrinfo>(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
}

void	WebServer::_initCreateMasterSocket(
	const Server &currentServer, const struct addrinfo &hints)
{

	int	newSocket;

	newSocket = Socket(AF_INET, SOCK_STREAM, 0);
	_masterSockets.push_back(newSocket);
	Fcntl(newSocket, F_SETFL, O_NONBLOCK);
	Getaddrinfo(currentServer.getServerName(), currentServer.getListen(), &hints, &_gaiPtr);
	Setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR);
	Bind(newSocket, _gaiPtr->ai_addr, _gaiPtr->ai_addrlen);
	Listen(newSocket, 1024); // ici taille max de la backlog

	freeaddrinfo(_gaiPtr);
	_gaiPtr = NULL;
}

void	WebServer::_initMasterSocket()
{
	struct addrinfo hints;

	std::vector<Server>::iterator it, ite;
	it	= _config.servers.begin();
	ite	= _config.servers.end();

	_initReferenceSocket(hints);
	for (; it != ite; it++)
		_initCreateMasterSocket(*it, hints);
}

size_t	WebServer::_initGetPollSize()
{
	unsigned int fds;

	try {
		fds = safeAddition(_config.clientMax, _config.servers.size());
	}
	catch (std::out_of_range &e)
	{
		throw (ExceptionBadInit("Max clients + number of servers is out of range"));
	}
	if (fds > FD_SETSIZE)
	{
		if (_config.servers.size() > FD_SETSIZE)
			throw (ExceptionBadInit("Server number is exceeding FD_SETSIZE"));
		fds = FD_SETSIZE - _config.servers.size();
	}
	return (static_cast<int>(fds));
}

void	WebServer::_initPollFds()
{
	_pollSize = _initGetPollSize();
	_pollfds = new pollfd[_pollSize];
	t_bzero(_pollfds, sizeof(*_pollfds) * _pollSize);
	for (size_t i = 0; i < _masterSockets.size(); i++)
	{
		_pollfds[i].fd = _masterSockets[i];
		_pollfds[i].events = POLLIN;
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

void	WebServer::run()
{
	_connectionManager.loop();
}

/*
*	TODO: REFACTO
*	Some variables (in comanager.init) should be in its constructor + not in webserv ?
*/
void	WebServer::init()
{
	_initMasterSocket();
	_initPollFds();
	_connectionManager.init(_pollSize, _masterSockets, _pollfds);
}

// CONSTRUCTORS

/*
*	TODO: REFACTO
*	reserve in the constructor ?
*/
WebServer::WebServer(const WebServerConfig &config) : 
_config(config), _gaiPtr(NULL), _connectionManager(_config), _pollfds(NULL), _pollSize(0)
{
}

WebServer::~WebServer()
{
	for (unsigned int i = 0; i < _masterSockets.size(); i++)
		safeClose(_masterSockets[i]);
	delete [] _pollfds ;
	if (_gaiPtr)
		freeaddrinfo(_gaiPtr);
}


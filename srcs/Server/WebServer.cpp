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

	currentServer.display();
	int	newSocket;

	newSocket = Socket(AF_INET, SOCK_STREAM, 0);
	_masterSockets.push_back(newSocket);
	_serverCount++;
	Fcntl(newSocket, F_SETFL, O_NONBLOCK);
	Getaddrinfo(currentServer.getServerName(), currentServer.getListen(), &hints, &_gaiPtr);
	Setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR);
	Bind(newSocket, _gaiPtr->ai_addr, _gaiPtr->ai_addrlen);
	Listen(newSocket, BACKLOG_SIZE); // ici taille max de la backlog

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

int	WebServer::_initGetPollSize()
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
		std::cerr << "Max clients + number of servers exceeds FD_SETSIZE (" << FD_SETSIZE << ").\n";
		std::cerr << "Max client number set to " << fds << "\n";
	}

	std::cout << "The program is initialized with " << _serverCount << " servers." << "\n";
	std::cout << "It can listen to " << fds << " clients" << std::endl;

	//TODO: this is false, needs to be corrected
	return (static_cast<int>(fds));
}

void	WebServer::_initPollFds()
{
	_pollSize = _initGetPollSize();
	_pollfds = new pollfd[_pollSize];
	t_bzero(_pollfds, sizeof(*_pollfds) * _pollSize);
	for (int i = 0; i < _serverCount; i++)
	{
		_pollfds[i].fd = _masterSockets[i];
		_pollfds[i].events = POLLIN;
		_size++;
		_tempSize++;
	}
}

void	WebServer::_processConnections(int masterSocket, const Server &currServ)
{
	int	socketFd;

	while (_size < _pollSize)
	{
		// if (_size >= _pollSize)
		// {
		// 	std::cout << YELLOW << "Size= " << _size << " | POLLSIZE= " << _pollSize << std::endl;
		// 	std::cerr << "Cannot accept client request, server capacity exceeded" << END << std::endl;
		// 	break ;
		// }
		socketFd = accept(masterSocket, 0, 0);
		if (socketFd == -1)
			break ;
		std::cout << YELLOW << "Connected to client " << _size << " on fd " << socketFd << std::endl;
		_pollfds[_size].fd = socketFd; 
		_pollfds[_size].revents = POLLIN;
		_size++;
		_tempSize++;
		
		Request req(currServ);
		std::pair<int, Request> p = std::make_pair(socketFd, req);
		_requestsMap.insert(p);
	}
}

void	WebServer::_loopUpdate()
{
	if (_size != _tempSize) // not finished + don't like
	{
		for (int i = 0; i < _tempSize; i++)
		{
			if (_pollfds[i].fd == -1)
			{
				std::cerr << RED ITALIC << "Cleaned a client: " << i << END << "\n";
				for (int j = i; j < _size - 1; j++)
				{
					_pollfds[j] = _pollfds[j + 1];
				}

				_tempSize--;
				i--;
			}
		}
	}
}

void	WebServer::loop()
{
	bool processing_req;
	bool sending_resp;

	while (1) 
	{

		processing_req = false;
		sending_resp = false;

		_awaitRequests();
		std::cerr << BLUE ITALIC << "\n--> The server received a request, loop initiated. " << END << "\n";
		for (int i = 0; i < _size; i++)
		{
			pollfd &currRequest = _pollfds[i];
			if (i < _serverCount)
			{
				std::cout << RED << "\nListening to connections on server: " << i + 1 << " (fd: " << currRequest.fd << ")" << std::endl;
				_processConnections(_masterSockets[i], _config.servers[i]);
			}
			else if (currRequest.revents & POLLIN)
			{
				if (!processing_req)
				{
					std::cout << RED << "\nStarting to process requests: " << std::endl;
					processing_req = true;
				}
				std::cout << YELLOW << "Processing request from fd " << currRequest.fd << std::endl;
				_processRequest(currRequest);
			}
			else if (currRequest.revents & POLLOUT)
			{
				if (!sending_resp)
				{
					std::cout << RED << "\nStarting to send responses: " << std::endl;
					sending_resp = true;
				}
				_sendResponse(currRequest);
			}
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
	if (_requestsMap.find(request.fd) == _requestsMap.end())
		throw (ExceptionNull());
	Request currRequest = _requestsMap.at(request.fd);

	currRequest.recvRequest(request.fd);
	// std::cout << BLUE BOLD <<  currRequest.getRawRequest() << END << "\n";
	if (currRequest.isRequestComplete() == false)
		return ;
	//si pas fini de read la request return ;
	currRequest.parseRequest(currRequest.getRawRequest());
	
	Response resp(currRequest, currRequest.getServer());

	std::pair<int, Response> r = std::make_pair(request.fd, resp); // TODO: choose an other way or keep 
	_responsesMap.insert(r);

	request.events = POLLOUT; // on set le fd ready to send
}

void	WebServer::_sendResponse(pollfd &request)
{
	std::cout << "REQUEST FD " << request.fd << "\n";
	if (_responsesMap.find(request.fd) == _responsesMap.end())
		throw (ExceptionBadInit());
	Response &currResponse = _responsesMap.at(request.fd);

	std::string chunckedStr = currResponse.getChunckResp(SEND_SIZE);
	int bytesSent = send(request.fd, chunckedStr.c_str(), chunckedStr.size(), 0);
	// if (bytesSent == -1)
	// ;
	currResponse.updateBytesSent(bytesSent);
	if (currResponse.isSent())
	{
		request.revents = 0;
		std::cout << RED ITALIC << "Closing fd " << request.fd << END << std::endl;
		_responsesMap.erase(_responsesMap.find(request.fd)); // I THINK PROBLEM HERE
		close(request.fd);
		request.fd = -1;
		_size--;
	}
}

void	WebServer::init()
{
	_initMasterSocket();
	_initPollFds();
}

void	WebServer::_awaitRequests()
{
	poll(_pollfds, _size, TIMEOUT);
}

// CONSTRUCTORS

WebServer::WebServer(const WebServerConfig &config) : 
_config(config), _serverCount(0), _tempSize(0), _size(0), _pollfds(NULL), _pollSize(0), _gaiPtr(NULL)
{}

// DESTRUCTOR

WebServer::~WebServer()
{
	for (unsigned int i = 0; i < _masterSockets.size(); i++)
		close(_masterSockets[i]);
	delete [] _pollfds ;
	if (_gaiPtr)
		freeaddrinfo(_gaiPtr);
	std::cout << "Destructor called for class WebServer" << std::endl;
}

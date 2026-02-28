#include "WebServer.hpp"
#include "Request.hpp"
#include "Response.hpp"

# include "errno.h"

void	WebServer::_initMasterSocket()
{
	struct addrinfo *gai;
	struct addrinfo hints;
	int				tmp = 1;

	t_bzero<struct addrinfo>(&hints, sizeof(hints));
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
	_size++;
	_tempSize++;
}

void	WebServer::_processConnections()
{
	int	socketFd;

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
		Request req(_config.servers.back());
		std::pair<int, Request> p = std::make_pair(socketFd, req);
		_requestsMap.insert(p);
		_size++;
		_tempSize++;
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
	while (1) // TODO deal with error
	{
		_awaitRequests();
		for (int i = 0; i < _size; i++)
		{
			pollfd &currRequest = _pollfds[i];
			if (currRequest.fd == _masterSocket)
				_processConnections();
			else if (currRequest.revents & POLLIN)
				_processRequest(currRequest);
			else if (currRequest.revents & POLLOUT)
				_sendResponse(currRequest);
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
	Request currRequest = _requestsMap.at(request.fd);

	currRequest.recvRequest(request.fd);
	std::cout << BLUE BOLD <<  currRequest.getRawRequest() << END << "\n";
	if (currRequest.isRequestComplete() == false)
		return ;
	//si pas fini de read la request return ;
	currRequest.parseRequest(currRequest.getRawRequest());
	
	Response resp(currRequest, _config.servers.back()); // TODO: passer le bon server

	SentResponse sentresp;
	sentresp.rawResponse = resp.getResponse(sentresp);
	sentresp.bytesSent = 0;
	std::pair<int, SentResponse> r = std::make_pair(request.fd, sentresp); // TODO: choose an other way or keep 
	_responsesMap.insert(r);

	request.events = POLLOUT; // on set le fd ready to send
}

void	WebServer::_sendResponse(pollfd &request)
{
	SentResponse &currResponse = _responsesMap.at(request.fd);
	const char *res = currResponse.rawResponse.c_str() + currResponse.bytesSent; // TODO: dynamic
	int contentLength = stringToInt(extractContentLength(currResponse.rawResponse));
	int remainingSize = (contentLength + currResponse.headerLength) - currResponse.bytesSent;

	std::cout << GREEN BOLD ITALIC << res << END << "\n";
	currResponse.bytesSent += send(request.fd, res, (remainingSize < SEND_SIZE ? remainingSize : SEND_SIZE), 0);
	std::cout << ORANGE << "SENT " << currResponse.bytesSent << "\n";
	std::cout << "OVER  " << contentLength + currResponse.headerLength << "\n";
	std::cout <<  "REMAINING " << currResponse.bytesSent - (contentLength + currResponse.headerLength) << END"\n";
	//WHAT TO DO IF CLIENT DISCONNECT ??? 
	if (currResponse.bytesSent == contentLength + currResponse.headerLength)
	{
		_responsesMap.erase(request.fd);
		close(request.fd);
		request.fd = -1;
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

WebServer::WebServer(const WebServerConfig &config) : _config(config), _tempSize(0), _size(0) {}

// DESTRUCTOR

WebServer::~WebServer()
{
	close(_masterSocket);
	// std::cout << "Destructor called for class WebServer" << std::endl;
}

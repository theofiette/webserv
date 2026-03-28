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
		std::cerr << "Max clients + number of servers exceeds FD_SETSIZE (" << FD_SETSIZE << ").\n";
		std::cerr << "Max client number set to " << fds << "\n";
	}

	std::cerr << "The program is initialized with " << _masterSockets.size() << " servers." << "\n";
	std::cerr << "It can listen to " << fds << " clients" << std::endl;

	//TODO: this is false, needs to be corrected
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

// void	WebServer::_processRequest(pollfd &request)
// {
// 	if (_requestsMap.find(request.fd) == _requestsMap.end())
// 		throw (ExceptionRuntime("Can't find corresponding request."));
// 	Request &currRequest = _requestsMap.at(request.fd);

// 	currRequest.receiveRequestChunk(request.fd);
// 	if (currRequest.isRequestComplete() == false)
// 		return ;

// 	std::cout << GREEN << currRequest.getRawRequest() << std::endl << END;
// 	currRequest.parseRequest(currRequest.getRawRequest());
	// std::cout << currRequest.getHeader() << std::endl;
	// for (std::vector<char>::const_iterator it = currRequest.getBody().begin(); it != currRequest.getBody().end(); it++)
	// 	std::cout <<  *it;


// 	Session &currSession = _sessionManager.accessSession(currRequest.getCookieValue("SessionId"), currRequest.getServerIndex());

// 	currSession.display();


// 	Response resp(currRequest, currRequest.getServer(), currSession);

// 	std::pair<int, Response> r = std::make_pair(request.fd, resp); // TODO: choose an other way or keep
	
// 	_responsesMap.insert(r);

// 	request.events = POLLOUT; // on set le fd ready to send
// }

// void	WebServer::_sendResponse(pollfd &request)
// {
// 	if (_responsesMap.find(request.fd) == _responsesMap.end())
// 		throw (ExceptionRuntime("Can't find corresponding response."));
// 	Response &currResponse = _responsesMap.at(request.fd);

// 	std::string chunckedStr = currResponse.getChunckResp(SEND_SIZE);
// 	int bytesSent;
// 	std::cout << chunckedStr << "\n";
// 	try
// 	{
// 		bytesSent = Send(request.fd, chunckedStr);
// 	}
// 	catch (std::exception &e){
// 		std::cerr << RED "Write Error in sendResponse." << std::endl;
// 		_closeConnection(request, _pollfds);
// 		return ;
// 	}
// 	currResponse.updateBytesSent(bytesSent);
// }

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
	_sessionManager.setServerCount(_config.servers.size());
	_sessionManager.initServerSessions();
	_initMasterSocket();
	_initPollFds();
	_connectionManager.init(_pollSize, _masterSockets, _pollfds);
}

// void	WebServer::_awaitRequests()
// {
// 	int	socketsCount;

// 	socketsCount = _masterSockets.size() + _connectionManager.size();
// 	// std::cerr << "DEBUG: polling on " << socketsCount << "fds" << std::endl;
// 	poll(_pollfds, socketsCount, TIMEOUT); //TODO : wrap for -1
// }

// void	WebServer::_closeConnection(pollfd &request, pollfd *fds)
// {
// 	(void)request;
// 	(void)fds;
// 	// _decrement = true;
// 	// request.revents = 0;
// 	// std::cout << RED ITALIC << "Closing fd " << request.fd << END << std::endl;
// 	// if (_responsesMap.find(request.fd) != _responsesMap.end())
// 	// 	_responsesMap.erase(_responsesMap.find(request.fd));
// 	// if (_requestsMap.find(request.fd) != _requestsMap.end())
// 	// 	_requestsMap.erase(_requestsMap.find(request.fd));
// 	// close(request.fd);
// 	// request = fds[_connectionsCount - 1];
// 	// _connectionsCount--;
// }

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
	std::cout << "Destructor called for class WebServer" << std::endl;
}


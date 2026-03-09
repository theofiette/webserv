# pragma once

# include "webserv.hpp"
# include <vector>
# include <map>
# include <sstream>

# include "Server.hpp"
# include "WebServerConfig.hpp"
# include "Session.hpp"
# include "signal.hpp"

class Request;
class Response;
class SessionManager;

struct SentResponse;

class WebServer
{
	private:
		WebServerConfig				_config;

		std::vector<int>			_masterSockets;
		int							_serverCount;
		int							_tempSize;
		int							_size;
		pollfd 						*_pollfds;
		int							_pollSize;

		struct addrinfo 			*_gaiPtr;

		std::map<int, Request> 		_requestsMap;
		std::map<int, Response> 	_responsesMap;

		SessionManager				_sessionManager;

		void	_initMasterSocket();
		void	_initReferenceSocket(struct addrinfo &hints);
		void	_initCreateMasterSocket(const Server &currentServer, const struct addrinfo &hints);

		void	_initPollFds();
		int		_initGetPollSize();

		void	_awaitRequests();
		void	_processConnections(int masterSocket, const Server &currServ);
		void	_processRequest(pollfd &);
		void	_sendResponse(pollfd &);	

		void	_loopUpdate();

		WebServer();

	public:
		void	init();
		void	loop();
		void	display();

		WebServer(const WebServerConfig &config);
		~WebServer();

};

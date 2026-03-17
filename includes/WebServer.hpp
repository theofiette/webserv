# pragma once

# include "webserv.hpp"
# include <vector>
# include <map>
# include <sstream>

# include "Server.hpp"
# include "WebServerConfig.hpp"
# include "SessionManager.hpp"
# include "signal.hpp"

class Request;
class Response;

struct SentResponse;

class WebServer
{
	private:
		WebServerConfig				_config;

		std::vector<int>			_masterSockets;
		int							_serverCount;
		int							_connectionsCount ;
		pollfd 						*_pollfds;
		int							_pollSize;

		struct addrinfo 			*_gaiPtr;

		bool						_decrement;
		std::map<int, Request> 		_requestsMap;
		std::map<int, Response> 	_responsesMap;

		SessionManager				_sessionManager;

		void	_initMasterSocket();
		void	_initReferenceSocket(struct addrinfo &hints);
		void	_initCreateMasterSocket(const Server &currentServer, const struct addrinfo &hints);

		void	_initPollFds();
		int		_initGetPollSize();

		void	_awaitRequests();
		void	_closeConnection(pollfd &, pollfd *);
		void	_processConnections(int masterSocket, const Server &currServ, unsigned int serverIndex);
		void	_processRequest(pollfd &);
		void	_sendResponse(pollfd &);	

		WebServer();

	public:
		void	init();
		void	loop();
		void	display();

		void	display_connection();
		void	display_pollfds();


		WebServer(const WebServerConfig &config);
		~WebServer();

};

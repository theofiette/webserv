# pragma once

# include "webserv.hpp"
# include <vector>
# include <map>

# include "Server.hpp"
# include "signal.hpp"
# include "WebServerConfig.hpp"
#include <sstream>

class Request;
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
		std::map<int, SentResponse> _responsesMap;


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

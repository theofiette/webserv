# pragma once

# include "webserv.hpp"
# include <vector>
# include <map>

# include "Server.hpp"
# include "WebServerConfig.hpp"
#include <sstream>

class Request;

typedef struct SentResponse 
{
	std::string rawResponse;
	int			bytesSent;
	int			headerLength;
}	SentResponse;

class WebServer
{
	private:
		WebServerConfig				_config;

		int							_masterSocket;	
		int							_tempSize;
		int							_size;
		pollfd 						_pollfds[MAX_CLIENTS];
		std::map<int, Request> 		_requestsMap;
		std::map<int, SentResponse> _responsesMap;

		void	_initMasterSocket();
		void	_initPollFds();
		void	_awaitRequests();
		void	_processConnections();
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

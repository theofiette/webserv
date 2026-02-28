# pragma once

# include "webserv.hpp"
# include <vector>
# include "Server.hpp"
# include "WebServerConfig.hpp"

#include <sstream>

class WebServer
{
	private:

		WebServerConfig		_config;

		int					_masterSocket;
		int					_tempSize;
		int					_size;
		pollfd 				_pollfds[MAX_CLIENTS];

		void	_initMasterSocket();
		void	_initPollFds();
		void	_awaitRequests();
		void	_processConnections();
		void	_processRequest(pollfd &request);
		void	_loopUpdate();

		WebServer();
	
	public:

		void	loop();
		void	display();

		WebServer(const WebServerConfig &config);
		~WebServer();

};

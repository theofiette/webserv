# pragma once

# include "webserv.hpp"
# include <vector>
# include <map>
# include <sstream>

# include "WebServerConfig.hpp"
# include "ConnectionManager.hpp"

# include "Server.hpp"

# include "SessionManager.hpp"
# include "signal.hpp"

class Request;
class Response;
class ConnectionManager;

struct SentResponse;

class WebServer
{
	private:

		WebServerConfig				_config;

		std::vector<int>			_masterSockets;
		struct addrinfo 			*_gaiPtr;

		ConnectionManager			_connectionManager;


		// size_t						_connectionsCount;
		pollfd 						*_pollfds;
		size_t						_pollSize;

		// SessionManager				_sessionManager;

		void	_initMasterSocket();
		void	_initReferenceSocket(struct addrinfo &hints);
		void	_initCreateMasterSocket(const Server &currentServer, const struct addrinfo &hints);

		void	_initPollFds();
		size_t	_initGetPollSize();

		//TODO : REFACTO //////////////////////////////////////////////////////
		// Move dans connection manager
		void	_checkTimedOutConnections();
		void	_processTimedOutConnections(pollfd&, const Request &);

		WebServer();

	public:

		void	init();
		void	run();
		void	display();

		// void	display_connection();
		// void	display_pollfds();

		WebServer(const WebServerConfig &config);
		~WebServer();

};

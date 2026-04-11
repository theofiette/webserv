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

		pollfd 						*_pollfds;
		size_t						_pollSize;

		void	_initMasterSocket();
		void	_initReferenceSocket(struct addrinfo &hints);
		void	_initCreateMasterSocket(const Server &currentServer, const struct addrinfo &hints);

		void	_initPollFds();
		size_t	_initGetPollSize();

		WebServer();

	public:

		void	init();
		void	run();
		void	display();

		WebServer(const WebServerConfig &config);
		~WebServer();

};

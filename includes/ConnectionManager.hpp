#pragma once

# include "WebServerConfig.hpp"
# include "Connection.hpp"

# include <vector>
# include <stack>

typedef unsigned long size_t ;

class ConnectionManager
{

	private:

		size_t				_maxConnections;
		std::vector<int>	_masterSockets;
		pollfd				*_pollfds;

		std::vector<Connection>								_connections;
		std::stack< std::vector<Connection>::iterator >		_to_close;

		// TODO : outside  ////////////////////////////////////////////////////
		const std::vector<Server>	&_servers;
		///////////////////////////////////////////////////////////////////////

		void	_reserve(size_t n);

		void	_awaitEvents();
		size_t	_setupPollfds();
		void	_checkOpenConnections();

		// void	_openConnection(int fd);
		// TMP ////////////////////////////////////////////////////////////////
		void	_openConnection(int fd, const Server &tmp_serv, const unsigned int serverId);
		///////////////////////////////////////////////////////////////////////

		void	_processConnections();
		void	_checkCloseConnections();
		void	_closeConnection(std::vector<Connection>::iterator it);

		void	_clean();

		ConnectionManager();

	public:

	// Variables const a set dans un constructeur ??
		void init(
			size_t maxConnections, const std::vector<int> masterSockets, pollfd *pollfds);

		size_t	size();
		void	loop();
		
		ConnectionManager(const WebServerConfig &config);
		~ConnectionManager();
};

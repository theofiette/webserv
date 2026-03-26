# include <sys/types.h>
# include <sys/socket.h>

# include "wrapper.hpp"
# include "ConnectionManager.hpp"

void	ConnectionManager::init(
	size_t maxConnections, const std::vector<int> masterSockets, pollfd *pollfds)
{
	_maxConnections = maxConnections;
	_masterSockets = masterSockets;
	_pollfds = pollfds;

	_reserve(maxConnections);
	if (_maxConnections == 0 || _masterSockets.empty() || pollfds == 0)
		throw (ExceptionBadInit("Couldn't init ConnectionManager correctly"));
}

/*
*	TODO: REFACTO
*	Verifier si maxConnection a la bonne valeur (nombre de client / nombre de fd)
*	Skip step si meme pollfd ou si pas de modif
*	Virer i et remplacer par masterSocket.size ?
*/
size_t	ConnectionManager::_setupPollfds()
{
	size_t	i;
	size_t	j;

	i = 0;
	j = 0;	
	while (i + j < _maxConnections)
	{
		if (i < _masterSockets.size())
		{
			// _pollfds[i].fd = _masterSockets[i];
			// _pollfds[i].events = POLLIN;
			i++;
		}
		else if (j < _connections.size())
		{
			_pollfds[i + j].fd = _connections[j].getFd();
			if (_connections[j].getState() == CS_SENDING)
			{
				_pollfds[i + j].events = POLLOUT;
				std::cerr << "SET AS POLLOUT\n";
			}
			else
				_pollfds[i + j].events = POLLIN;

			_pollfds[i + j].revents = 0;
			j++;
		}
		else
			return (i + j);
	}
	// std::cerr << RED << "PollTab: ";
	// for (size_t k = 0; k < i + j; k++)
	// 	std::cerr << _pollfds[i].fd << " | ";
	// std::cerr << std::endl;
	return (_maxConnections);
}

/*
*	Creer une connection, remplir certaine choses ?? 
*	Je pense a requete, response, session etc
*/
void	ConnectionManager::_openConnection(int fd, const Server &tmp_serv, unsigned int tmp_int)
{
	// Connection	newConnection(fd);
	Connection	newConnection(fd, tmp_serv, tmp_int);

	_connections.push_back(newConnection);
}

/*
*	TODO: IMPORTANT
*/

void	ConnectionManager::_checkCloseConnections()
{
	// if (!_to_close.empty())
	// 	std::cerr << RED << "DEBUG: starting closing process on stack of size " << _to_close.size() << END << std::endl;
	while (!_to_close.empty())
	{
		_closeConnection(_to_close.top());
		_to_close.pop();
	}
}

/*
*	TODO:	REFACTO
*	trouver moyen de fermer fd dans destructeur de connection
*/
void	ConnectionManager::_closeConnection(std::vector<Connection>::iterator it)
{
	std::cerr << RED << "closing connection on fd " << it->getFd() << END << std::endl;
	close(it->getFd());
	_connections.erase(it);
}

void	ConnectionManager::_checkOpenConnections()
{
	int newSocket;

	for (size_t i = 0; i < _masterSockets.size(); i++)
	{
		while (_connections.size() < _maxConnections)
		{
			newSocket = accept(_masterSockets[i], 0, 0);
			if (newSocket < 0)
				break ;
			_openConnection(newSocket, _servers[i], i);
			std::cerr << RED << "DEBUG: opening connection on fd " << newSocket << END << "\n";
		}
	}
}

void	ConnectionManager::_processConnections()
{
	std::vector<Connection>::iterator it, ite;
	size_t		index;
	size_t		i;


	it	= _connections.begin();
	ite	= _connections.end();
	i = 0;
	for (; it != ite; it++)
	{
		Connection	&currentConnection = *it;

		// std::cout << RED BOLD << "coNENCTION NUMBER : " << i << END << std::endl; 

		index = _masterSockets.size() + i;
		std::cout << "revent is : " << _pollfds[index].revents << "\n";
		currentConnection.processTmp(_pollfds[index].revents);
		if (currentConnection.getState() == CS_CLOSE)
		{
			_to_close.push(it);
		}

	}
}

/*
*	TODO : REFACTO
*	Some variables in ConnectionManager should be const = constructeur ?;
*/
void	ConnectionManager::_awaitEvents()
{
	size_t socketCount;

	// socketCount = _masterSockets.size() + _connections.size();
	socketCount = _setupPollfds();
	// std::cerr << "DEBUG: polling on " << socketsCount << "fds" << std::endl;
	poll(_pollfds, socketCount, TIMEOUT); //TODO : wrap for -1
}

void	ConnectionManager::loop()
{
	while (1)
	{
		_awaitEvents();
		_checkOpenConnections();
		_processConnections();
		_checkCloseConnections();
	}
}

void	ConnectionManager::_reserve(size_t n)
{
	std::cerr << "DEBUG: reserved_connections for " << n << " possibles connections" << std::endl;
	_connections.reserve(n);
}

size_t	ConnectionManager::size()
{
	return (_connections.size());
}

//	CONSTRUCTOR & DESTRUCTOR //////////////////////////////////////////////////

ConnectionManager::ConnectionManager(const WebServerConfig &config) : _servers(config.servers)
{
}

ConnectionManager::~ConnectionManager()
{
	// std::cerr << "DEBUG: Destructor called for class ConnectionManager\n";
}

///////////////////////////////////////////////////////////////////////////////
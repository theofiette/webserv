
#include "Server.hpp"
#include "Location.hpp"

# define SERVER_DEF_PORT	8080
# define SERVER_DEF_NAME	"localhost"
# define SERVER_DEF_SEND	false
# define SERVER_DEF_BODY	2048
# define SERVER_DEF_ERROR	"html/errors/default_error.html"

void	Server::registerLocation(string &name, Location &loc)
{
	std::pair<string, Location>	pair;

	pair.first = name;
	pair.second = loc;
	_locations.insert(pair);
}

void	Server::display() const
{
	std::cout << "\n___DISPLAYING SERVER INFOS___\n\n";
	std::cout << "Listening to port: " << _listen << "\n";
	std::cout << "Server name is: " << _serverName << "\n";
	std::cout << "The option to send file is set as: " << std::boolalpha << _sendFile << "\n";
	std::cout << "The max client body size is: " << _clientBodyMax << "\n";

	{
		std::cout << "\n___Errors: \n\n";
		mc_is_ite it = _errors.begin();
		mc_is_ite ite = _errors.end();
		for (; it != ite; it++)
			std::cout << "Error " << it->first << " -> " << it->second << "\n";
	}
	{
		std::cout << "\n___Locations: \n\n";
		mc_sL_ite it = _locations.begin();
		mc_sL_ite ite = _locations.end();
		for (; it != ite; it++)
		{
			std::cout << "Location " << it->first << ": \n";
			it->second.display();
		}
	}

	std::cout << "____________________________\n" << std::endl;
}

// CONSTRUCTORS

Server::Server() :
_listen (SERVER_DEF_PORT), _serverName(SERVER_DEF_NAME),
_sendFile(SERVER_DEF_SEND), _clientBodyMax(SERVER_DEF_BODY)
{
	(void)_listen, (void)_sendFile, (void)_clientBodyMax; //TODO delete
	_errors.insert(std::pair<int, string>(0, SERVER_DEF_ERROR));
	// std::cout
	// 	<< "Default constructor called for class Server" << std::endl;
}

// DESTRUCTOR

Server::~Server()
{
	// std::cout
	// 	<< "Destructor called for class Server" << std::endl;
}

// GETTERS

std::map<string, Location>& Server::getLocations()
{
	return (_locations);
}

std::map<int, std::string>&  Server::getErrMapServer(){
	return (_errors);
}

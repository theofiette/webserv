#include "Server.hpp"
#include "Location.hpp"
# define SERVER_DF_PORT		"8080"
# define SERVER_DF_NAME		"127.0.0.1"
# define SERVER_DF_SEND		false
# define SERVER_DF_BODY		2048
# define SERVER_DF_ERROR	"html/errors/default_error.html"

string	Server::_df_listen			= SERVER_DF_PORT;
string	Server::_df_serverName		= SERVER_DF_NAME;
bool	Server::_df_sendFile		= SERVER_DF_SEND;
int		Server::_df_clientBodyMax	= SERVER_DF_BODY;
string	Server::_df_errorPath		= SERVER_DF_ERROR;

// std::map<string, Location> &	Server::getLocations()
// {
// 	return (_locations);
// }

bool	Server::registerLocation(string &name, Location &loc)
{
	std::pair<string, Location>	pair;

	pair.first = name;
	pair.second = loc;
	if (_locations.find(name) != _locations.end())
		return (false);
	_locations.insert(pair);
	return (true);
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

// METHODS
//------------------------------------------------

bool	Server::checkErrorsValidity()
{
	std::string	path;
	mc_is_ite	it, ite;

	it	= _errors.begin();
	ite	= _errors.end();

	for (; it != ite; it++)
	{
		path = it->second;
		std::cout << "Checking error file: " << path << "\n";
		if (access(path.c_str(), R_OK))
			return (false);
	}
	return (true);
}

void	Server::checkDefaultLocation()
{
	if (!_locations.size() || _locations.find("/") == _locations.end())
		_locations.insert(std::pair<string, Location>("/", Location()));
}

void	Server::clear()
{
	_listen = _df_listen;
	_serverName = _df_serverName;
	_sendFile = _df_sendFile;
	_clientBodyMax = _df_clientBodyMax;
	_errorPage = _df_errorPath;
	_errors = std::map<int, string>();
	_locations = std::map<string, Location>();
	_errors.insert(std::pair<int, string>(0, _errorPage));
}

void	Server::insertError(int code, string path)
{
	std::pair<int, string> new_pair(code, path);

	if (_errors.find(code) == _errors.end())
		_errors.insert(new_pair);
	else
		_errors.find(code)->second = path;
}

//------------------------------------------------


// SETTER // GETTER
//------------------------------------------------

void	Server::setListen(string value)
{
	_listen = value;
}

void	Server::setServerName(string value)
{
	_serverName = value;
}

void	Server::setSendfile(bool value)
{
	_sendFile = value;
}

void	Server::setClientBodyMax(int value)
{
	_clientBodyMax = value;
}

void	Server::setErrorPath(string value)
{
	_errorPage = value;
}

void	Server::set_df_listen(string value)
{
	Server::_df_listen = value;
}

void	Server::set_df_serverName(string value)
{
	Server::_df_serverName = value;
}

void	Server::set_df_sendFile(bool value)
{
	Server::_df_sendFile = value;
}
void	Server::set_df_clientBodyMax(int value)
{
	Server::_df_clientBodyMax = value;
}
void	Server::set_df_errorPath(string value)
{
	Server::_df_errorPath = value;
}

//------------------------------------------------

// CONSTRUCTORS

Server::Server() :
_listen (_df_listen), _serverName(_df_serverName),
_sendFile(_df_sendFile), _clientBodyMax(_df_clientBodyMax)
{
	_errors.insert(std::pair<int, string>(0, _df_errorPath));
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

const std::map<string, Location>&	Server::getLocations() const
{
	return (_locations);
}

const std::map<int, std::string>&	Server::getErrorsServer() const
{
	return (_errors);
}

// const std::string&					Server::getServerName() const
// {
// 	return (_serverName);
// }

// const int&							Server::getServerPort() const
// {
// 	return (_listen);
// }


int Server::getMaxBody() const
{
	return (_clientBodyMax);
}

const string Server::getServerName() const
{
	return (_serverName);
}

const string Server::getListen() const
{
	return (_listen);
}

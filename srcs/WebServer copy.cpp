
#include "WebServer.hpp"

# include "errno.h"

void	WebServer::_initMasterSocket()
{
	struct addrinfo *gai;
	struct addrinfo hints;
	int				tmp = 1;

	t_bzero< struct addrinfo >(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
	_masterSocket = socket(AF_INET, SOCK_STREAM, 0); // TODO check protocol
	//TODO gai_error
	if (
		_masterSocket == -1														||
		fcntl(_masterSocket, F_SETFL, O_NONBLOCK) == - 1 						||
		getaddrinfo(NULL, LOCALHOST, &hints, &gai) == -1 						||
		setsockopt(_masterSocket, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int))	||
		bind(_masterSocket, gai->ai_addr, gai->ai_addrlen) == -1				||
		listen(_masterSocket, MAX_CLIENTS) == -1)
		{
			std::cerr << strerror(errno) << std::endl;
			throw( ExceptionBadInit() );
		}
	freeaddrinfo(gai);
}

void	WebServer::_initPollFds()
{
	t_bzero(_pollfds, sizeof(_pollfds));
	_pollfds[0].fd = _masterSocket;
	_pollfds[0].events = POLLIN;
	_size++; // (set size at 1);
	_tempSize++;
}

void	WebServer::_processConnections()
{
	int	socketFd;

	std::cout << "IN REQUEST CONNECTION \n\n";
	while(1)
	{
		socketFd = accept(_masterSocket, 0, 0); // TODO check args
		if (socketFd < 0)
		{
			// TODO error here ?
			break ;
		}
		_pollfds[_size].fd = socketFd; 
		_pollfds[_size].events = POLLIN;
		_size++;
	}
}

void	WebServer::_loopUpdate()
{
	if (_size != _tempSize) // not finished + don't like
	{
		for (int i = 0; i < _size; i++)
		{
			if (_pollfds[i].fd == -1)
			{
				for (int j = i; j < _size - 1; j++)
				{
					_pollfds[j] = _pollfds[j + 1];
				}
				_tempSize--;
				i--;
			}
		}
	}
	_size = _tempSize;
}

void	WebServer::loop()
{
	pollfd currRequest;

	while (1) // TODO deal with error
	{
		_awaitRequests();
		std::cout << "OUT OF POLL\n";
		for (int i = 0; i < _size; i++)
		{
			currRequest = _pollfds[i];
			// if (!currRequest.revents)
			// 	continue ;
			// if (currRequest.revents != POLLIN)
			// {
			// 	// TODO error here ?
			// 	break;
			// }
			if (currRequest.fd == _masterSocket)
				_processConnections();
			else
				_processRequest(currRequest);
		}
		_loopUpdate();
	}
}

void	WebServer::_processRequest(pollfd &request)
{
	(void)request;
}

void	WebServer::_awaitRequests()
{
	std::cout << "AWAIT\n";
	std::cout << _tempSize << "\n";
	std::cout << _size << "\n";
	poll(_pollfds, _size, TIMEOUT);
}

typedef std::string::const_iterator cstr_ite;
#include <sstream>

string	formatRemoveComment(string line)
{
	unsigned long length;

	length = line.find('#');
	if (length != std::string::npos)
		line = line.substr(0, line.find('#'));
	return (line);
}

string	formatStopEndline(string line)
{
	string::iterator	it = line.begin();
	string				newline;
	while (*it)
	{
		if (*it == ';')
		{
			newline += " ;";
			break ;
		}
		else
			newline += *it;
		it++;
	}
	return (newline);
}

string	formatRemoveSpaces(string line)
{
	std::istringstream	stream(line);
	std::string			word;
	std::string			newline;

	while (stream >> word)
	{
		if(!newline.empty())
			newline += ' ';
		newline += word;
	}
	return (newline);
}

string	formatSplitBracket(string line)
{
	string::iterator	it = line.begin();
	string				newline;
	while (*it)
	{
		if (*it == '{' || *it == '}')
		{
			newline += ' ';
			newline += *it;
			newline += ' ';
		}
		else
			newline += *it;
		it++;
	}
	return (newline);
}

// string	formatSplitComma(string line)
// {
// 	string::iterator	it = line.begin();
// 	string				newline;
// 	while (*it)
// 	{
// 		if (*it == ';')
// 			newline += " ; ";
// 		else
// 			newline += *it;
// 		it++;
// 	}
// 	return (newline);
// }

void	stream_to_string(std::stringstream &iss, string &str, bool keep)
{
	string	word;
	string	newline;

	if (keep == true)
		newline += str;
	while (iss >> word)
	{
		if (!newline.empty())
			newline += " ";
		newline += word;
	}
	str = newline;
}

string	format(string line)
{
	line = formatRemoveComment(line);
	line = formatSplitBracket(line);
	line = formatStopEndline(line);
	line = formatRemoveSpaces(line);

	return (line);
}

void	WebServer::_configStateNone(parseState &state, string &line, int &bracket, int lineCount)
{
	std::stringstream	stream(line);
	string				word;

	stream >> word;
	if (!bracket && word == "local")
		state = LOCAL;
	else if (!bracket && word == "global")
		state = GLOBAL;
	else if (!bracket && word == "server")
		state = SERVER;
	else 
	{
		std::cerr << "Error (_configStateNone) on line " << lineCount <<  std::endl;
		throw (ExceptionBadInit());
	}
	stream_to_string(stream, line, false);
}

void	WebServer::_configCheckBrackets(parseState &state, string &line, int &bracket, int lineCount, Server &initServ)
{
	std::stringstream	stream(line);
	string				word;
	string				newline;

	stream >> word;
	if (word == "{")
	{
		bracket++;
		if (bracket == 2 && state == LOCATION)
		{}
		else if (bracket != 1 || state == NONE)
		{
			std::cerr << "Error (_configCheckBrackets) on line " << lineCount <<  std::endl;
			throw (ExceptionBadInit());
		}
		if (!(stream >> word))
			word = "";
	}
	if (word == "}")
	{
		bracket--;
		if (bracket == 1 && state == LOCATION)
		{
			std::cout << "End of location\n";
			state = SERVER;
		}
		else if (bracket != 0 || state == NONE)
		{
			std::cerr << "Error (_configCheckBrackets) on line " << lineCount <<  std::endl;
			throw (ExceptionBadInit());
		}
		else
		{
			if (state == SERVER)
			{
				if (initServ.getLocations().size() == 0 || initServ.getLocations().find("/") == initServ.getLocations().end())
					initServ.getLocations().insert(std::pair<string, Location>("/", Location()));
				std::cout << "Pushing server !\n";
				_servers.push_back(initServ);
				std::cout << "Size is now " << _servers.size() << "\n";
				initServ.clear();
			}
			state = NONE;
		}
		if (!(stream >> word))
			word = "";
	}
	newline += word;
	stream_to_string(stream, newline, true);
	line = newline;
}

void	WebServer::_configStateLocal(parseState &state, string &line, int lineCount, Server &initServ)
{
	std::stringstream	stream(line);
	string				word;
	
	stream >> word;
	_checkParamValidity(word, state, lineCount);
	stream_to_string(stream, line, false);
	_configDispatchParam(word, line, lineCount, initServ);
}

int	WebServer::_configSafeStoi(string &str, int lineCount)
{
	std::istringstream	iss(str);
	int					res;

	iss >> res;
	if (iss.fail())
	{
		std::cerr << "Error (_configSafeStoi) on line " << lineCount <<  std::endl;
		throw (ExceptionBadInit());
	}
	return (res);
}

void	WebServer::_configStateGlobal(parseState &state, string &line, int lineCount, Server &initServ)
{
	std::stringstream	stream(line);
	string				word;
	
	stream >> word;
	_checkParamValidity(word, state, lineCount);
	stream_to_string(stream, line, false);
	_configDispatchParam(word, line, lineCount, initServ);
}

bool	WebServer::_configSafeStob(string &str, int lineCount)
{
	if (str == "on")
		return (true);
	if (str == "off")
		return (false);
	std::cerr << str << ": ";
	std::cerr << "Error (_configSafeStob) on line " << lineCount <<  std::endl;
	throw (ExceptionBadInit());
	return (true);
}


//TODO safe values
//TODO enums ??

void	WebServer::_configDispatchParam(string &word, string &line, int lineCount, Server &initServer)
{
	std::stringstream	stream(line);
	string				next_word;
	string				third_word;

	stream >> next_word;
	if (word == "max_clients")
		_clientsMax = _configSafeStoi(next_word, lineCount);
	if (word == "server_listen")
		Server::set_df_listen(_configSafeStoi(next_word, lineCount));
	if (word == "redir_path")
		Redir::set_df_redirPath(next_word);
	if (word == "server_name")
		Server::set_df_serverName(next_word);
	if (word == "server_sendfile")
		Server::set_df_sendFile(_configSafeStob(next_word, lineCount));
	if (word == "server_clientbodymax")
		Server::set_df_clientBodyMax(_configSafeStoi(next_word, lineCount));
	if (word == "server_error")
		Server::set_df_errorPath(next_word);
	if (word == "location_method")
		Location::set_df_method(_configSafeStob(next_word, lineCount));
	if (word == "location_root")
		Location::set_df_root(next_word);
	if (word == "location_index")
		Location::set_df_index(next_word);
	if (word == "location_autoindex")
		Location::set_df_autoIndex(_configSafeStob(next_word, lineCount));
	if (word == "redir_valid")
		Redir::set_df_valid(_configSafeStob(next_word, lineCount));
	if (word == "redir_errorcode")
		Redir::set_df_errorCode(_configSafeStoi(next_word, lineCount));
	if (word == "listen")
		initServer.setListen(_configSafeStoi(next_word, lineCount));
	if (word == "sendfile")
		initServer.setSendfile(_configSafeStob(next_word, lineCount));
	if (word == "error_page")
	{
		stream >> third_word;
		if (third_word == ";")
		{
			initServer.insertError(0, next_word);
			stream_to_string(stream, line, false);
			return ;
		}
		else
			initServer.insertError(_configSafeStoi(next_word, lineCount), third_word);
	}
	if (word == "methods")
	{
		m_sL_ite 	ite;
		int			i;
		ite = (initServer.getLocations().find(initServer.lastLocation));

		do
		{
			i = METHODS_COUNT ;
			if (next_word == ";")
			{
				stream_to_string(stream, line, false);
				return ;
			}
			if (next_word == "GET")
				i = GET;
			if (next_word == "POST")
				i = POST;
			if (next_word == "DELETE")
				i = DELETE;
			if (i == METHODS_COUNT)
			{
				std::cerr << "Error (_configDispatchParam) on line " << lineCount <<  std::endl;
				throw (ExceptionBadInit());
			}
			ite->second.setMethod(true, i);
		}
		while (stream >> next_word);
	}
	if (word == "root")
	{
		m_sL_ite 	ite;
		ite = (initServer.getLocations().find(initServer.lastLocation));

		ite->second.setRoot(next_word);
	}
	if (word == "index")
	{
		m_sL_ite 	ite;
		ite = (initServer.getLocations().find(initServer.lastLocation));

		ite->second.setIndex(next_word);
	}
	if (word == "autoindex")
	{
		m_sL_ite 	ite;
		ite = (initServer.getLocations().find(initServer.lastLocation));

		ite->second.setAutoIndex(_configSafeStob(next_word, lineCount));
	}
	if (word == "return") 
	{
		m_sL_ite 	ite;
		ite = (initServer.getLocations().find(initServer.lastLocation));
		ite->second.getRedir().setValid(true);
		if (next_word == ";")
		{
			stream_to_string(stream, line, false);
			return ;
		}
		else
		{
			ite->second.getRedir().setErrorCode(_configSafeStoi(next_word, lineCount));
			stream >> third_word;
			ite->second.getRedir().setRedirPath(third_word);
		}
	}
	stream >> next_word;
	if (next_word != ";")
	{
		std::cerr << "Error (_configDispatchParam) on line " << lineCount <<  std::endl;
		throw (ExceptionBadInit());
	}
	stream_to_string(stream, line, false);
}


void	WebServer::_configStateServer(parseState &state, string &line, int &bracket, int lineCount, Server &initServer)
{
	std::stringstream	stream(line);
	string				word;
	
	stream >> word;

	if (word == "location")
	{
		if (bracket == 1)
		{
			Location	newLocation;

			state = LOCATION;
			stream >> word;
			initServer.registerLocation(word, newLocation);
			initServer.lastLocation = word;

			if (!(stream >> word) || word != "{")
			{
				std::cerr << "Error (_configStateServer) on line " << lineCount <<  std::endl;
				throw (ExceptionBadInit());
			}
			else
				bracket ++;
			stream_to_string(stream, line, false);
			return ;
		}
		else
		{
			std::cerr << "Error (_configStateServer) on line " << lineCount <<  std::endl;
			throw (ExceptionBadInit());
		}
	}
	else
	{
		_checkParamValidity(word, state, lineCount);
		stream_to_string(stream, line, false);
		_configDispatchParam(word, line, lineCount, initServer);
	}
}

void	initLookUpParamValidity(std::map<string, int>	&table)
{
	table.insert(std::pair<string, int>("max_clients", 			LOCAL));
	table.insert(std::pair<string, int>("redir_path", 			GLOBAL));
	table.insert(std::pair<string, int>("server_name", 			GLOBAL));
	table.insert(std::pair<string, int>("server_listen", 		GLOBAL));
	table.insert(std::pair<string, int>("server_sendfile", 		GLOBAL));
	table.insert(std::pair<string, int>("server_clientbodymax",	GLOBAL));
	table.insert(std::pair<string, int>("server_error",			GLOBAL));
	table.insert(std::pair<string, int>("location_method",		GLOBAL));
	table.insert(std::pair<string, int>("location_root",		GLOBAL));
	table.insert(std::pair<string, int>("location_index",		GLOBAL));
	table.insert(std::pair<string, int>("location_autoindex",	GLOBAL));
	table.insert(std::pair<string, int>("redir_valid",			GLOBAL));
	table.insert(std::pair<string, int>("redir_errorcode",		GLOBAL));
	table.insert(std::pair<string, int>("redir_path",			GLOBAL));
	table.insert(std::pair<string, int>("sendfile", 			SERVER));
	table.insert(std::pair<string, int>("error_page", 			SERVER));
	table.insert(std::pair<string, int>("client_body_max", 		SERVER));
	table.insert(std::pair<string, int>("listen", 				SERVER));
	table.insert(std::pair<string, int>("name", 				SERVER));
	table.insert(std::pair<string, int>("methods", 				LOCATION));
	table.insert(std::pair<string, int>("root", 				LOCATION));
	table.insert(std::pair<string, int>("index", 				LOCATION));
	table.insert(std::pair<string, int>("autoindex", 			LOCATION));
	table.insert(std::pair<string, int>("return", 				LOCATION));
}

// stream in this function ?
void	WebServer::_checkParamValidity(string param, parseState state, int lineCount)
{
	static std::map<string, int>	table;

	if (table.empty())
		initLookUpParamValidity(table);
	if (table.at(param) != state)
	{
		std::cerr << "Error (_checkParamValidity) on line " << lineCount <<  std::endl;
		throw (ExceptionBadInit());
	}
}

void	WebServer::_configStateLocation(parseState &state, string &line, int lineCount, Server &initServ)
{

	std::stringstream	stream(line);
	string				word;
	
	stream >> word;
	_checkParamValidity(word, state, lineCount);
	stream_to_string(stream, line, false);
	_configDispatchParam(word, line, lineCount, initServ);
}

void	WebServer::_configDispatch(parseState &state, string &line, int &bracket, int lineCount, Server &initServ)
{
	switch (state)
	{
		case (NONE):
			_configStateNone(state, line, bracket, lineCount);
			break;
		case (LOCAL):
			_configStateLocal(state, line, lineCount, initServ);
			break;
		case (GLOBAL):
			_configStateGlobal(state, line, lineCount, initServ);
			break ;
		case (SERVER):
			_configStateServer(state, line, bracket, lineCount, initServ);
			break ;
		case (LOCATION):
			_configStateLocation(state, line, lineCount, initServ);
			break ;
		default:
		{
			std::cerr << "Error (_configDispatch) on line " << lineCount <<  std::endl;
			throw (ExceptionBadInit());
		}
	}
}

void	WebServer::_initConfig(const string &configFilePath)
{
	string			line;
	std::ifstream	file(configFilePath.c_str(), std::ios_base::in);
	parseState		state = NONE;
	int				brackets = 0;
	int				lineCount = 0;
	Server			initServ;

	if (!file.good())
		throw (ExceptionBadInit());

	while (!std::getline(file, line).eof())
	{
		lineCount++;
		line = format(line);
		while (!line.empty())
		{
			_configCheckBrackets(state, line, brackets, lineCount, initServ);
			if (!line.empty())
				_configDispatch(state, line, brackets, lineCount, initServ);
		}
	}
}

// CONSTRUCTORS

WebServer::WebServer(const WebServerConfig &config)
{
	_config = config;
}

// DESTRUCTOR

WebServer::~WebServer()
{
	close(_masterSocket);
	// std::cout << "Destructor called for class WebServer" << std::endl;
}

// WEBSERVERCONFIG

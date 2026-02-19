
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
	Request r(request.fd);

	close(request.fd);
	request.fd = -1;
}

void	WebServer::_awaitRequests()
{
	std::cout << "AWAIT\n";
	std::cout << _tempSize << "\n";
	std::cout << _size << "\n";
	poll(_pollfds, _size, TIMEOUT);
}

typedef std::string::const_iterator cstr_ite;

// void	WebServer::Config::_registerListened(const string &param)
// {
// 	cstr_ite			it 	= param.begin();
// 	cstr_ite			ite = param.begin();
// 	int					offset;
// 	std::pair<int, int>	listened;

// 	while (it != ite)
// 	{
// 		offset = param.find(':');
// 	}
// }

// void	WebServer::Config::_registerClientMax(const string &param)
// {
	
// }

// void	WebServer::Config::_registerErrorPage(const string &param)
// {
	
// }

// void	WebServer::Config::_registerBodyMax(const string &param)
// {

// }

// void	WebServer::Config::_registerParam(int type, const string &param)
// {
// 	switch (type)
// 	{
// 		case LISTENED:
// 			_registerListened(param);
// 			break ;
// 		case CLIENT_MAX:
// 			_registerClientsMax(param);
// 			break ;
// 		case ERROR_PAGE:
// 			_registerErrorPage(param);
// 			break ;
// 		case BODY_MAX:
// 			_registerBodyMax(param);
// 			break ;
// 		default:
// 			std::cerr << "TEMP MESSAGE: config not processed : " << param;
// 	}
// }

// void	WebServer::_initConfig(const string &configFilePath)
// {
// 	std::ifstream		file(configFilePath.c_str(), std::ifstream::in);
// 	string				buffer;
// 	std::vector <std::pair <string, bool> >	rules;
	
// 	//TODO : automation
// 	rules.push_back(std::pair<string, int> ("port:",	false));
// 	rules.push_back(std::pair<string, int> ("connec:",	false));
// 	rules.push_back(std::pair<string, int> ("error:",	false));
// 	rules.push_back(std::pair<string, int> ("reqbody:", false));

// 	if (file.fail())
// 		throw (ExceptionBadInit()); //TODO
// 	while (!std::getline(file, buffer).eof())
// 	{
// 		std::vector< std::pair<string, bool> >::iterator	ite = rules.begin();

// 		for (int i = 0; i < CONFIG_RULE; i++)
// 		{
// 			string rhs = ite->first;
// 			if (buffer.find(rhs) == 0)
// 			{
// 				_config._registerParam(i, buffer.substr(rhs.size(), string::npos));
// 				ite->second = true;
// 				break ;
// 			}
// 			ite++;
// 		}
// 	}
// }


void	WebServer::_temp_config()
{
	Server		temp_S;
	Location 	temp_L;
	string		L_name("/");

	temp_S.registerLocation(L_name, temp_L);
	temp_S.display();
	_servers.push_back(temp_S);
}

// CONSTRUCTORS

WebServer::WebServer(const std::string &configFilePath) : 
_masterSocket(0), _tempSize(0), _size(0)
{
	(void)configFilePath;
	_temp_config();
	// _initConfig(configFilePath);
	_initMasterSocket();
	_initPollFds();
	std::cout
		<< "Default constructor called for class WebServer" << std::endl;
}

// DESTRUCTOR

typedef std::vector<int>::iterator v_ite;

WebServer::~WebServer()
{
	close(_masterSocket);
	// v_ite b = _masterSockets.begin();
	// v_ite e = _masterSockets.end();

	// for (; b != e; b++)
	// 	close(*b);
	std::cout << "Destructor called for class WebServer" << std::endl;
}

// WEBSERVERCONFIG

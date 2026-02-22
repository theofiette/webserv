# pragma once

# include "webserv.hpp"

# include <vector>
# include <map>


# include "Server.hpp"

class WebServer
{
private:

	int					_clientsMax;
	std::vector<Server>	_servers;

	int					_masterSocket;	
	int					_tempSize;
	int					_size;
	pollfd 				_pollfds[MAX_CLIENTS];

	std::map<int, std::string> _responses;

	// void	_initConfig(const string &configFilePath);
	void	_initMasterSocket();
	void	_initPollFds();
	void	_awaitRequests();
	void	_processConnections();
	void	_processRequest(pollfd &);
	void	_sendResponse(pollfd &);	

	void	_loopUpdate();

	WebServer();
	
public:

	//TEMPORARY //TODO delete
	void	_temp_config(); // This function gives a default configurated webserv

	void loop();

	WebServer(const string &configFilePath);
	~WebServer();

	// WebServer(const WebServer &other);	//Copy constructor
	// WebServer	&operator=(const WebServer &other); //Assignement operator
};


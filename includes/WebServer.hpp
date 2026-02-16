# pragma once

# include "webserv.hpp"

class WebServer
{
private:

	int		_masterSocket;
	int		_temp_size;
	int		_size;
	pollfd _pollfds[MAX_CLIENTS];

	void	_initMasterSocket();
	void	_initPollFds();

	void	_awaitRequests();
	void	_processConnections();
	void	_processRequest(pollfd &request);
	void	_loopUpdate();

public:

	void loop();

	WebServer();
	~WebServer();

	// WebServer(const WebServer &other);	//Copy constructor
	// WebServer	&operator=(const WebServer &other); //Assignement operator
};

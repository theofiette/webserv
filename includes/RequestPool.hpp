# pragma once

# include <iostream>
# include <stack>

# include "webserv.hpp"

class ARequest;

class RequestPool
{
private:

	int		_masterSocket;
	int		_temp_size;
	int		_size;
	pollfd _pollfds[MAX_CLIENTS];

	void	RequestPool::_initMasterSocket();
	void	RequestPool::_initPollFds();

	void	RequestPool::_awaitRequests();
	void	RequestPool::_processConnections();
	void	RequestPool::_processRequest(pollfd &request);
	void	RequestPool::_loopUpdate();

public:

	void RequestPool::loop();

	RequestPool();
	~RequestPool();

	// RequestPool(const RequestPool &other);	//Copy constructor
	// RequestPool	&operator=(const RequestPool &other); //Assignement operator
};

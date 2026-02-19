# pragma once

# include "macros.hpp"
# include <poll.h>

class Request;
class Response;

class RequestPool
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

	RequestPool();
	~RequestPool();

	// RequestPool(const RequestPool &other);	//Copy constructor
	// RequestPool	&operator=(const RequestPool &other); //Assignement operator
};

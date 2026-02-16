
#include "RequestPool.hpp"

void	RequestPool::_initMasterSocket()
{
	struct addrinfo *gai;
	struct addrinfo hints;

	t_bzero< struct addrinfo >(&hints, sizeof(hints));
    hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    //    hints.ai_protocol = 0;          /* Any protocol */
    //    hints.ai_canonname = NULL;
    //    hints.ai_addr = NULL;
    //    hints.ai_next = NULL;
	_masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(_masterSocket, F_SETFL, O_NONBLOCK);
	getaddrinfo(NULL, LOCALHOST, &hints, &gai);
	// sockaddr_in serverAddress;
	// serverAddress.sin_family = AF_INET;
	// serverAddress.sin_addr.s_addr = INADDR_ANY;
	// serverAddress.sin_port = htons(LOCALHOST);
	bind(_masterSocket, gai->ai_addr, gai->ai_addrlen);
	listen(_masterSocket, MAX_CLIENTS);
}

void	RequestPool::_initPollFds()
{
	t_bzero(_pollfds, sizeof(_pollfds));
	_pollfds[0].fd = _masterSocket;
	_pollfds[0].events = POLLIN;
	_size++; // (set size at 1);
}

void	RequestPool::_processConnections()
{
	int	socketFd;

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
	}
	_size++;
	_temp_size++;
}

void	RequestPool::_loopUpdate()
{
	if (_size != _temp_size) // not finished + don't like
	{
		for (int i = 0; i < _size; i++)
		{
			if (_pollfds[i].fd == -1)
			{
				for (int j = i; j < _size - 1; j++)
				{
					_pollfds[j] = _pollfds[j + 1];
				}
				i--;
				_size--;
			}
		}
	}
	_temp_size = _size;
}

void	RequestPool::loop()
{
	pollfd currRequest;

	while (1) // TODO deal with error
	{
		_awaitRequests();
		std::cout << "OUT OF POLL";
		for (int i = 0; i < _temp_size; i++)
		{
			currRequest = _pollfds[i];
			if (currRequest.revents)
				continue ;
			if (currRequest.revents != POLLIN)
			{
				// TODO error here ?
				break;
			}
			if (currRequest.fd == _masterSocket)
				_processConnections();
			else
				_processRequest(currRequest);
		}
		_loopUpdate();
	}
}

void	RequestPool::_processRequest(pollfd &request)
{
	// TODO : send fd (int) to Ilyes

	char buffer[2000] = {0};
	std::string buf;

	while (recv(request.fd, buffer, sizeof(buffer) - 1, 0) > 0)\
	{
		buf += buffer;
		t_bzero(&buffer, sizeof(buffer));
	}
	std::cout << buf << "\n";
	close(request.fd);
	request.fd = -1;
}

void	RequestPool::_awaitRequests()
{
	std::cout << "AWAIT\n";
	poll(_pollfds, _temp_size, TIMEOUT);
}

// CONSTRUCTORS

RequestPool::RequestPool() : 
_masterSocket(0), _temp_size(0), _size(0)
{
	_initMasterSocket();
	_initPollFds();
	std::cout
		<< "Default constructor called for class RequestPool" << std::endl;
}


// DESTRUCTOR

RequestPool::~RequestPool()
{
	if (_masterSocket)
		close(_masterSocket);
	std::cout
		<< "Destructor called for class RequestPool" << std::endl;
}

# pragma once

# include <vector>


struct WebServerInit
{
	std::vector<int>			_masterSockets;
	int							_serverCount;
	struct addrinfo 			*_gaiPtr;
};
// #include "../includes/webserv.hpp"
#include "webserv.hpp"
#include <vector>




int main()
{
	RequestPool	requests;

	pollfd pollfds[MAX_CLIENTS];
		
	int newSocket = 0;
	int errCheck = 0;

	while(!errCheck)
	{
		requests.loop();
	}
}

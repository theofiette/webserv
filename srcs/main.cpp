// #include "../includes/webserv.hpp"
#include "RequestPool.hpp"
#include "webserv.hpp"
#include <vector>
#include <csignal>
#include <cstdlib>

//TODO delete
int	g_masterSocket = 0;

void sigHandler(int sig)
{
	if (g_masterSocket)
		close(g_masterSocket);
	exit(sig + 128);
}

int main()
{
	RequestPool	requests;

	signal(SIGINT, sigHandler);
	try
	{
		requests.loop();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

// #include "../includes/webserv.hpp"
#include "webserv.hpp"
#include <vector>
#include <csignal>
#include <cstdlib>

//TODO delete
int	g_masterSocket = 0;

void exceptionManager(std::exception *e)
{
	int	errorCode = 1;

	if (dynamic_cast<ExceptionSignal *>(e))
		errorCode = dynamic_cast<ExceptionSignal *>(e)->getSig();
	std::cerr << e->what() << std::endl;
	exit(errorCode);
}

int main()
{
	RequestPool	requests;

	try
	{
		requests.loop();
	}
	catch (std::exception &e)
	{
		exceptionManager(&e);
	}
	return (0);
}
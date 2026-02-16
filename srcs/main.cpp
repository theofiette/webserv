// #include "../includes/webserv.hpp"
#include "WebServer.hpp"
#include "webserv.hpp"
#include <vector>
#include <csignal>
#include <cstdlib>

void sigHandler(int sig)
{
	throw (ExceptionSignal(128 + sig));
}

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
	try
	{
		WebServer	requests;

		signal(SIGINT, sigHandler);
		requests.loop();
	}
	catch (std::exception &e)
	{
		exceptionManager(&e);
	}
	return (0);
}
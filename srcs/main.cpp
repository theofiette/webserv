// #include "../includes/webserv.hpp"
#include "webserv.hpp"
#include "WebServer.hpp"
#include <vector>
#include <csignal>
#include <cstdlib>

//TODO delete
void sigHandler(int sig)
{
	std::cerr << "\n";
	throw (ExceptionSignal(128 + sig));
}

void exceptionManager(std::exception *e)
{
	int	errorCode = 1;

	if (dynamic_cast<ExceptionSignal *>(e))
		errorCode = dynamic_cast<ExceptionSignal *>(e)->getSig();
	std::cerr << BOLD << e->what() << END << std::endl;
	exit(errorCode);
}

#define CONFIG_PATH "./config"

int main(int argc, char *argv[])
{	
	signal(SIGINT, sigHandler);
	try
	{
		std::string	configFilePath = (argc > 1 ? argv[1] : CONFIG_PATH);
		WebServer	webServer(configFilePath);

		webServer.loop();
	}
	catch (std::exception &e)
	{
		exceptionManager(&e);
	}
	return (0);
}
// #include "../includes/webserv.hpp"
#include "ConfigParser.hpp"
#include "webserv.hpp"
#include "WebServer.hpp"
#include <vector>
#include <csignal>
#include <cstdlib>

int g_sig;

void exceptionManager(std::exception *e, int &returnValue)
{
	if (dynamic_cast<ExceptionSignal *>(e))
	{
		std::cerr << "\n";
		returnValue = g_sig;
	}
	std::cerr << BOLD RED << e->what() << END << std::endl;
}

#define CONFIG_PATH "./config/default.conf"

int main(int argc, char *argv[])
{	
	int returnValue = 0;

	signal(SIGINT, sigHandlerDefault);
	signal(SIGPIPE, SIG_IGN);
	try
	{
		std::string		configFilePath = (argc > 1 ? argv[1] : CONFIG_PATH);
		ConfigParser	parser;
		WebServer		webServer(parser.parseFile(configFilePath));

		// webServer.display();
		webServer.init();
		webServer.run();
	}
	catch (std::exception &e)
	{
		exceptionManager(&e, returnValue);
	}
	return (returnValue);
}
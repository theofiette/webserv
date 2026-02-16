// #include "../includes/webserv.hpp"
#include "RequestPool.hpp"
#include "webserv.hpp"
#include <vector>

int main()
{
	RequestPool	requests;

	try
	{
		requests.loop();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

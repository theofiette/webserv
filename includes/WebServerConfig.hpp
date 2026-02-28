# pragma once

# include <vector>
# include "Server.hpp"

struct WebServerConfig
{
	int					clientMax;
	std::vector<Server>	servers;
};
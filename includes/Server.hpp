# pragma once

# include "webserv.hpp"
# include <map>
# include "Location.hpp"

typedef	std::map<int, string>::const_iterator		mc_is_ite;
typedef	std::map<string, Location>::const_iterator	mc_sL_ite;

class Server
{
	private:

		int		_listen;
		string	_serverName;
		bool	_sendFile;
		int		_clientBodyMax;

		std::map<int, string>		_errors;
		std::map<string, Location>	_locations;

		public:
		
		void	registerLocation(string &name, Location &loc); //TODO protect
		void	display() const;

		Server(); 						//Default constructor
		~Server();						//Destructor

		std::map<string, Location>& getLocations();
};

# pragma once

# include "webserv.hpp"
# include <map>
# include "Location.hpp"

typedef	std::map<int, string>::const_iterator		mc_is_ite;
typedef	std::map<string, Location>::const_iterator	mc_sL_ite;
typedef	std::map<string, Location>::iterator		m_sL_ite;

class Server
{
	private:

		static string	_df_listen;
		static string	_df_serverName;
		static int		_df_clientBodyMax;
		static string	_df_errorPath;

		string			_listen;
		string			_serverName;
		int				_clientBodyMax;
		string			_errorPage;

		std::map<int, string>		_errors;
		std::map<string, Location>	_locations;

		public:
		
		void		insertError(int code, string path);
		void		checkDefaultLocation();
		bool		checkErrorsValidity();
		bool		registerLocation(string &name, Location &loc);
		void		display() const;
		void		clear();

		// std::map<string, Location> &getLocations();

		static void	set_df_listen(string value);
		static void	set_df_serverName(string value);
		static void	set_df_clientBodyMax(int value);
		static void	set_df_errorPath(string value);

		void		setListen(string value);
		void		setServerName(string value);
		void		setClientBodyMax(int value);
		void		setErrorPath(string value);

		Server(); 						//Default constructor
		~Server();						//Destructor

		int								 	getMaxBody() const;
		const string						getListen() const;
		const string						getServerName() const;
		const std::map<string, Location>	&getLocations() const;
		const std::map<int, std::string>	&getServerErrors() const;

};

# pragma once

# include "webserv.hpp"

# include <vector>
// # include <utility>
# include <map>

class WebServer
{

	struct Config
	{
		public:
			std::vector< std::pair<int, int> >	listened;
			int									clientsMax;
			std::string							errorPage;
			int									bodyMax;

			enum type
			{
				LISTENED,
				CLIENT_MAX,
				ERROR_PAGE,
				BODY_MAX
			};

			void	_registerParam(int type, const string &param);

			Config();
			~Config();

		private:
			void	_registerListened(const string &param);
			void	_registerClientsMax(const string &param);
			void	_registerErrorPage(const string &param);
			void	_registerBodyMax(const string &param);
	};

private:

	struct Config		_config;
	std::vector<int>	_masterSockets;	
	int					_tempSize;
	int					_size;
	pollfd 				_pollfds[MAX_CLIENTS];

	void	_initConfig(const string &configFilePath);
	void	_initMasterSocket();
	void	_initPollFds();
	void	_awaitRequests();
	void	_processConnections();
	void	_processRequest(pollfd &request);
	void	_loopUpdate();

	WebServer();
	
public:

	void loop();

	WebServer(const string &configFilePath);
	~WebServer();

	// WebServer(const WebServer &other);	//Copy constructor
	// WebServer	&operator=(const WebServer &other); //Assignement operator
};


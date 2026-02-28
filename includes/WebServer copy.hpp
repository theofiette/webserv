# pragma once

# include "webserv.hpp"
# include <vector>
# include "Server.hpp"
# include "WebServerConfig.hpp"

#include <sstream>
enum	parseState {NONE, LOCAL, GLOBAL, SERVER, LOCATION};

class WebServer
{
	struct	Config
	{
		int					clientMax;
		std::vector<Server> servers;
	};

	private:

		WebServerConfig		_config;
		int					_clientsMax;
		std::vector<Server>	_servers;

		int					_masterSocket;
		int					_tempSize;
		int					_size;
		pollfd 				_pollfds[MAX_CLIENTS];

		void	_initMasterSocket();
		void	_initPollFds();
		void	_awaitRequests();
		void	_processConnections();
		void	_processRequest(pollfd &request);
		void	_loopUpdate();

		void	_initConfig(const string &configFilePath);

		int		_configSafeStoi(string &str, int lineCount);
		bool	_configSafeStob(string &str, int lineCount);

		void	_checkParamValidity(string param, parseState state, int lineCount);

		void	_configCheckBrackets(parseState &state, string &line, int &bracket, int lineCount, Server &initServ);

		void	_configStateNone(parseState &state, string &line, int &bracket, int lineCount);
		void	_configStateLocal(parseState &state, string &line, int lineCount, Server &initServ);
		void	_configStateGlobal(parseState &state, string &line, int lineCount, Server &initServ);
		void	_configStateServer(parseState &state, string &line, int &bracket, int lineCount, Server &initServ);
		void	_configStateLocation(parseState &state, string &line, int lineCount, Server &initServ);

		void	_configDispatch(parseState &state, string &line, int &bracket, int lineCount, Server &initServ);
		void	_configDispatchParam(string &word, string &line, int lineCount, Server &initServ);


		WebServer();
	
	public:

		//TEMPORARY //TODO delete
		void	_temp_config(); // This function sets a default configurated webserv
		void	loop();

		WebServer(const string &configFilePath);
		~WebServer();

};

# pragma once

# include "WebServerConfig.hpp"
# include "format.hpp"

enum	parseScope {NONE, LOCAL, GLOBAL, SERVER, LOCATION};

class ConfigParser
{
	private:

		WebServerConfig						_config;

		std::ifstream						_file;
		std::stringstream					_stream;
		std::string							_token;
		int									_lineCount;
		int									_bracketCount;
		parseScope							_scope;
		Server								_currentServer;
		std::pair<std::string, Location>	_Location;

		void	_parseError(const char *errorString);

		void	_openFile(const std::string &filePath);
		void	_formatLine(std::string &line);

		bool	_extractToken();

		void	_checkBrackets();
		void	_bracketOpen();
		void	_bracketClosed();

		void	_dispatchScope();
		void	_switchScope();
		void	_scopeServer();

		void	_setParameter();
		void	_lookUpParamValidity();
		void	_lookUpParamFunction();
		void	_initLookUpParamValidityTable(std::map<string, int> &table);
		void	_initLookUpParamFunctionTable(std::map<string, void (ConfigParser::*)(void)> &table);

		int		_safeStoi(std::string str);
		bool	_safeStob(std::string str);

		void	_PARAM_max_clients();
		void	_PARAM_server_listen();
		void	_PARAM_server_name();
		void	_PARAM_server_clientbodymax();
		void	_PARAM_server_error();
		void	_PARAM_location_method();
		void	_PARAM_location_root();
		void	_PARAM_location_index();
		void	_PARAM_location_cookie();
		void	_PARAM_location_autoindex();
		void	_PARAM_redir_valid();
		void	_PARAM_redir_errorcode();
		void	_PARAM_redir_path();
		void	_PARAM_upload();
		void	_PARAM_error_page();
		void	_PARAM_client_body_max();
		void	_PARAM_listen();
		void	_PARAM_name();
		void	_PARAM_methods();
		void	_PARAM_cgi();
		void	_PARAM_error();
		void	_PARAM_root();
		void	_PARAM_index();
		void	_PARAM_cookie();
		void	_PARAM_autoindex();
		void	_PARAM_return();

	public:

		WebServerConfig parseFile(const std::string &filePath);

		ConfigParser();
		~ConfigParser();
};
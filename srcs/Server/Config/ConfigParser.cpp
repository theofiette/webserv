
# include "ConfigParser.hpp"
#include <sys/stat.h>
int	ConfigParser::_safeStoi(std::string str)
{
	std::istringstream	iss(str);
	int					res;

	iss >> res;
	if (iss.fail())
		_parseError("Invalid parameter conversion.");
	if (res < 0)
		_parseError("Negative parameter.");
	return (res);
}

bool	ConfigParser::_safeStob(std::string str)
{
	if (str == "on")
		return (true);
	if (str == "off")
		return (false);
	_parseError("Invalid parameter conversion.");
	return (true);
}

void	ConfigParser::_parseError(const char *errorString)
{	
	std::string	output(errorString);

	if (_lineCount)
	{
		std::stringstream	stream;

		stream << " (line ";
		stream << _lineCount;
		stream << ")";
		output += stream.str();
	}
	throw (ExceptionBadInit(output));
}

void	ConfigParser::_openFile(const std::string &filePath)
{
	struct stat s;

	_file.open(filePath.c_str(), std::ios_base::in);
	if (_file.fail() || stat(filePath.c_str(), &s) == -1)
		_parseError("Could not properly open config file");
	if (!S_ISREG(s.st_mode))
		_parseError("Config path doesn't lead to a regular file");	
}

void	ConfigParser::_formatLine(std::string &line)
{
	formatStopAt(line, '#', false);
	formatStopAt(line, ';', true);
	formatAddSpacesAround(line, ';');
	formatAddSpacesAround(line, '{');
	formatAddSpacesAround(line, '}');
	formatRemoveConsecutiveSpaces(line);
}

void	ConfigParser::_bracketOpen()
{
	_bracketCount++;
	if (_bracketCount != 1 || _scope == NONE)
	{
		if (!(_bracketCount == 2 && _scope == LOCATION))
			_parseError("Invalid open bracket.");
	}
	_extractToken();
}

void	ConfigParser::_bracketClosed()
{
	_bracketCount--;
	if (_bracketCount == 1 && _scope == LOCATION)
	{
		if (!_currentServer.checkErrorsValidity())
			_parseError("Location has invalid error file.");
		_currentServer.registerLocation(_Location.first, _Location.second);
		_scope = SERVER;
	}
	else if (_bracketCount != 0 || _scope == NONE)
		_parseError("Invalid closed bracket.");
	else
	{
		if (_scope == SERVER)
		{
			_currentServer.checkDefaultLocation();
			if (!_Location.second.checkErrorsValidity())
				_parseError("Server has invalid error file.");
			_config.servers.push_back(_currentServer);
			// _currentServer.display();
		}
		_scope = NONE;
	}
	_extractToken();
}

void	ConfigParser::_checkBrackets()
{
	if (_token == "{")
		_bracketOpen();
	if (_scope == LOCATION && _bracketCount != 2)
		_parseError("Invalid location block.");
	if (_token == "}")
		_bracketClosed();

	if (_token == "{" || _token == "}")
		_checkBrackets();
}

bool	ConfigParser::_extractToken()
{
	if (!(_stream >> _token))
	{
		_token = "";
		return (false);
	}
	return (true);
}

void	ConfigParser::_switchScope()
{
	if (!_bracketCount)
	{
		if (_token == "local")
			_scope = LOCAL;
		else if (_token == "global")
			_scope = GLOBAL;
		else if (_token == "server")
		{
			_currentServer.clear();
			_scope = SERVER;
		}
		else
			_parseError("Invalid scope variable.");
	}
	else if (_token == "location" && _scope == SERVER && _bracketCount == 1)
	{
		_scope = LOCATION;
		_extractToken();
		_Location.first = _token;
		_Location.second.clear();
	}
	else
		_parseError("Invalid scope variable.");
	_extractToken();
}

void	ConfigParser::_lookUpParamValidity()	
{
	static	std::map<string, int>	table;

	if (table.empty())
		_initLookUpParamValidityTable(table);
	if (table.find(_token) == table.end() || table.at(_token) != _scope)
		_parseError("Invalid variable assignment.");
}

void	ConfigParser::_lookUpParamFunction()
{
	static	std::map<string, void (ConfigParser::*)()>	table;

	void (ConfigParser::*funcptr)();

	if (table.empty())
		_initLookUpParamFunctionTable(table);
	funcptr = table[_token];
	_extractToken();
	(this->*funcptr)();
	_extractToken();
}

void	ConfigParser::_setParameter()
{
	_lookUpParamValidity();
	_lookUpParamFunction();
	if (_token != ";")
		_parseError("Expected `;\'.");
	_extractToken();
}

void	ConfigParser::_scopeServer()
{
	if (_token == "location")
	{
		_switchScope();
		return ;
	}
	_setParameter();
}

void	ConfigParser::_dispatchScope()
{
	switch (_scope)
	{
		case (NONE):
			_switchScope();
			break ;
		case (SERVER):
			_scopeServer();
			break ;
		case (LOCAL):
		case (GLOBAL):
		case (LOCATION):
			_setParameter();
	}
}

/*
*	This function is the starting point of the Config Parser process.
*
*	After its opening, the configuration file is parsed as follow:
*	1) Every line is formated (empty lines are ignored) and fed into a buffer.
*	2) Tokens are then individually extracted from the buffer and analyzed as :
*		bracket OR variable in a scope context.
*/
WebServerConfig ConfigParser::parseFile(const std::string &filePath)
{
	string			line;

	_openFile(filePath);
	while (!std::getline(_file, line).eof())
	{
		_lineCount++;
		_formatLine(line);
		_stream.str(line);
		_extractToken();
		while (!_token.empty())
		{
			_checkBrackets();
			if (!_token.empty())
				_dispatchScope();
		}
		_stream.clear();
	}
	if (!_config.servers.size())
		_parseError("Config ended without setting a server");
	_file.close();
	return (_config);
}

void	ConfigParser::_PARAM_max_clients()
{
	_config.clientMax = _safeStoi(_token);
}

void	ConfigParser::_PARAM_server_listen()
{
	if (_safeStoi(_token))
	Server::set_df_listen(_token);
}

void	ConfigParser::_PARAM_server_name()
{
	Server::set_df_serverName(_token);
}

void	ConfigParser::_PARAM_server_clientbodymax()
{
	Server::set_df_clientBodyMax(_safeStoi(_token));
}

void	ConfigParser::_PARAM_server_error()
{
	Server::set_df_errorPath(_token);
}

void	ConfigParser::_PARAM_location_method()
{
	Location::set_df_method(_safeStob(_token));
}

void	ConfigParser::_PARAM_location_root()
{
	Location::set_df_root(_token);
}

void	ConfigParser::_PARAM_location_index()
{
	Location::set_df_index(_token);
}

void	ConfigParser::_PARAM_location_cookie()
{
	Location::set_df_cookie(_safeStob(_token));
}

void	ConfigParser::_PARAM_location_autoindex()
{
	Location::set_df_autoIndex(_safeStob(_token));
}

void	ConfigParser::_PARAM_redir_valid()
{
	Redir::set_df_valid(_safeStob(_token));
}

void	ConfigParser::_PARAM_redir_errorcode()
{
	Redir::set_df_errorCode(_safeStoi(_token));
}

void	ConfigParser::_PARAM_redir_path()
{
	Redir::set_df_redirPath(_token);
}

void	ConfigParser::_PARAM_upload()
{
	_Location.second.setUpload(true);
	if (_token == ";")
	{
		_stream.clear();
		_stream.str(";");
	}
	else
	{
		_Location.second.setUploadDir(_token);
	}
}

void	ConfigParser::_PARAM_error_page()
{
	std::string	param;

	param = _token;
	_extractToken();
	if (_token == ";")
	{
		_stream.clear();
		_stream.str(";");
		_currentServer.insertError(0, param);
	}
	else
		_currentServer.insertError(_safeStoi(param), _token);
}

void	ConfigParser::_PARAM_client_body_max()
{
	_currentServer.setClientBodyMax(_safeStoi(_token));
}

void	ConfigParser::_PARAM_listen()
{
	_currentServer.setListen(_token);
}

void	ConfigParser::_PARAM_name()
{
	_currentServer.setServerName(_token);
}

void	ConfigParser::_PARAM_cgi()
{
	while (!_token.empty())
	{
		if (!_Location.second.registerCgi(_token))
			_parseError("Invalid CGI extension");
		_extractToken();
		if (_token == ";")
		{
			_stream.clear();
			_stream.str(";");
			return ;
		}
	}
}

void	ConfigParser::_PARAM_error()
{
	std::string	param;

	param = _token;
	_extractToken();
	if (_token == ";")
	{
		_stream.clear();
		_stream.str(";");
		_Location.second.insertError(0, param);
	}
	else
		_Location.second.insertError(_safeStoi(param), _token);
}

void	ConfigParser::_PARAM_methods()
{
	std::vector<int> vec;
	std::vector<int>::iterator it;
	bool value;

	while (1)
	{
		if (_token == "HEAD")
			vec.push_back(HEAD);
		else if (_token == "GET")
			vec.push_back(GET);
		else if (_token == "POST")
			vec.push_back(POST);
		else if (_token == "DELETE")
			vec.push_back(DELETE);
		else if ((_token == "on" || _token == "off") && !vec.empty())
		{
			value = _safeStob(_token);
			for (it = vec.begin(); it != vec.end(); it++)
				_Location.second.setMethod(value, *it);
			vec = std::vector<int>();
		}
		else
			_parseError("Invalid method.");
		_extractToken();
		if (_token == ";")
		{
			if (!vec.empty())
				_parseError("Invalid method.");
			_stream.clear();
			_stream.str(";");
			return ;
		}
	}
}

void	ConfigParser::_PARAM_root()
{
	_Location.second.setRoot(_token);
}

void	ConfigParser::_PARAM_index()
{
	_Location.second.setIndex(_token);
}

void	ConfigParser::_PARAM_cookie()
{
	_Location.second.setCookie(_safeStob(_token));
}

void	ConfigParser::_PARAM_autoindex()
{
	_Location.second.setAutoIndex(_safeStob(_token));
}

void	ConfigParser::_PARAM_return()
{
	_Location.second.getRedir().setValid(true);
	if (_token == ";")
	{
		_stream.clear();
		_stream.str(";");
		return ;
	}
	_Location.second.getRedir().setErrorCode(_safeStoi(_token));
	_extractToken();
	_Location.second.getRedir().setRedirPath(_token);
}

typedef std::pair<string, void (ConfigParser::*)(void)>	pairParam;

void	ConfigParser::_initLookUpParamFunctionTable(
	std::map<string, void (ConfigParser::*)(void)> &table)
{
	table.insert(pairParam("max_clients",			&ConfigParser::_PARAM_max_clients));
	table.insert(pairParam("server_listen",			&ConfigParser::_PARAM_server_listen));
	table.insert(pairParam("server_name", 			&ConfigParser::_PARAM_server_name));
	table.insert(pairParam("server_clientbodymax",	&ConfigParser::_PARAM_server_clientbodymax));
	table.insert(pairParam("server_error",			&ConfigParser::_PARAM_server_error));
	table.insert(pairParam("location_method",		&ConfigParser::_PARAM_location_method));
	table.insert(pairParam("location_root",			&ConfigParser::_PARAM_location_root));
	table.insert(pairParam("location_index",		&ConfigParser::_PARAM_location_index));
	table.insert(pairParam("location_cookie",		&ConfigParser::_PARAM_location_cookie));
	table.insert(pairParam("location_autoindex",	&ConfigParser::_PARAM_location_autoindex));
	table.insert(pairParam("redir_valid", 			&ConfigParser::_PARAM_redir_valid));
	table.insert(pairParam("redir_errorcode", 		&ConfigParser::_PARAM_redir_errorcode));
	table.insert(pairParam("redir_path", 			&ConfigParser::_PARAM_redir_path));
	table.insert(pairParam("error_page",			&ConfigParser::_PARAM_error_page));
	table.insert(pairParam("client_body_max",		&ConfigParser::_PARAM_client_body_max));
	table.insert(pairParam("listen",				&ConfigParser::_PARAM_listen));
	table.insert(pairParam("name",					&ConfigParser::_PARAM_name));
	table.insert(pairParam("methods",				&ConfigParser::_PARAM_methods));
	table.insert(pairParam("cgi",					&ConfigParser::_PARAM_cgi));
	table.insert(pairParam("error",					&ConfigParser::_PARAM_error));
	table.insert(pairParam("root",					&ConfigParser::_PARAM_root));
	table.insert(pairParam("index",					&ConfigParser::_PARAM_index));
	table.insert(pairParam("cookie",				&ConfigParser::_PARAM_cookie));
	table.insert(pairParam("autoindex",				&ConfigParser::_PARAM_autoindex));
	table.insert(pairParam("upload",				&ConfigParser::_PARAM_upload));
	table.insert(pairParam("return",				&ConfigParser::_PARAM_return));
}

void	ConfigParser::_initLookUpParamValidityTable(std::map<string, int> &table)
{
	table.insert(std::pair<string, int>("max_clients", 			LOCAL));
	table.insert(std::pair<string, int>("server_listen", 		GLOBAL));
	table.insert(std::pair<string, int>("server_name", 			GLOBAL));
	table.insert(std::pair<string, int>("server_clientbodymax",	GLOBAL));
	table.insert(std::pair<string, int>("server_error",			GLOBAL));
	table.insert(std::pair<string, int>("location_method",		GLOBAL));
	table.insert(std::pair<string, int>("location_root",		GLOBAL));
	table.insert(std::pair<string, int>("location_index",		GLOBAL));
	table.insert(std::pair<string, int>("location_cookie",		GLOBAL));
	table.insert(std::pair<string, int>("location_autoindex",	GLOBAL));
	table.insert(std::pair<string, int>("redir_valid",			GLOBAL));
	table.insert(std::pair<string, int>("redir_errorcode",		GLOBAL));
	table.insert(std::pair<string, int>("redir_path",			GLOBAL));
	table.insert(std::pair<string, int>("error_page", 			SERVER));
	table.insert(std::pair<string, int>("client_body_max", 		SERVER));
	table.insert(std::pair<string, int>("listen", 				SERVER));
	table.insert(std::pair<string, int>("name", 				SERVER));
	table.insert(std::pair<string, int>("methods", 				LOCATION));
	table.insert(std::pair<string, int>("cgi",	 				LOCATION));
	table.insert(std::pair<string, int>("error",	 			LOCATION));
	table.insert(std::pair<string, int>("root", 				LOCATION));
	table.insert(std::pair<string, int>("index", 				LOCATION));
	table.insert(std::pair<string, int>("cookie",				LOCATION));
	table.insert(std::pair<string, int>("autoindex", 			LOCATION));
	table.insert(std::pair<string, int>("upload", 				LOCATION));
	table.insert(std::pair<string, int>("return", 				LOCATION));
}

// CONSTRUCTORS

ConfigParser::ConfigParser() :
_lineCount(0), _bracketCount(0), _scope(NONE)
{}

// DESTRUCTOR

ConfigParser::~ConfigParser()
{}

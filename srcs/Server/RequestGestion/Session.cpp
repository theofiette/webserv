#include "webserv.hpp"
#include "Session.hpp"

std::pair<string, string> Session::_getCookieInfo(std::string str, bool &errorCheck)
{
	size_t offset		= str.find(' ');
	size_t equal_pos	= str.find('=');
	size_t comma_pos	= str.find(';');

	if (offset == std::string::npos || equal_pos == std::string::npos)
	{
		errorCheck = true;
		return (std::pair<string, string>());
	}
	std::string key		= str.substr(offset, equal_pos - offset);
	std::string value;
	if (comma_pos == std::string::npos)
		value	= str.substr(equal_pos + 1);
	else
		value	= str.substr(equal_pos + 1, comma_pos);
	return (std::pair<string, string>(key, value)) ;
}

void	Session::insertCookieFromString(std::string str)
{
	std::pair<string, string> 	cookie;
	bool						errorCheck;
	
	cookie = _getCookieInfo(str, errorCheck);
	if (errorCheck)
		return ;
	std::cout << "key is : " << cookie.first << " | " << "value is :" << cookie.second << std::endl;
}

SessionManager::SessionManager()
{

}

SessionManager::~SessionManager()
{

}

Session::Session()
{

}

Session::~Session()
{

}
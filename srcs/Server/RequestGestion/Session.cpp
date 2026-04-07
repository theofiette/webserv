#include "webserv.hpp"
#include "Session.hpp"

void Session::display()
{
	return ;
}

void Session::setConnected(bool value)
{
	_connected = value;
}

bool Session::isConnected() const
{
	return (_connected);
}

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
	std::pair<string, string> 			cookie;
	bool								errorCheck;
	
	cookie = _getCookieInfo(str, errorCheck);
	if (errorCheck)
		return ;
	insertCookieFromPair(cookie);
}

void	Session::insertCookieFromPair(std::pair<string, string> cookie)
{
	std::map<string, string>::iterator	it;

	it = _cookies.find(cookie.first);
	if (it == _cookies.end())
		_cookies.insert(cookie);
	else
		it->second = cookie.second;
}

const std::map<string, string> &	Session::getCookies() const
{
	return (_cookies);
}


const std::string &	Session::getId() const
{
	return (_id);
}

Session::Session(const std::string &id) : _id(id), _connected(false)
{
}

Session::~Session()
{
}
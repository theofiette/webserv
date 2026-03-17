#pragma once
#include "webserv.hpp"
#include "safemath.hpp"

class Session
{
	private:

		std::string 				_id;
		std::map<string, string>	_cookies;
		bool						_connected;

		std::pair<string, string>	_getCookieInfo(std::string str, bool &ref);

	public:

		void				insertCookieFromString(std::string str);
		void				insertCookieFromPair(std::pair<string, string> cookie);
		const std::string &	getId();
		bool				isConnected();
		void				setConnected(bool value);

		void				display();

		Session(const std::string &id); 	//Default constructor
		~Session();							//Destructor
};

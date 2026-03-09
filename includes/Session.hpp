#pragma once 


class SessionManager
{
	private:

	public:

	SessionManager();
	~SessionManager();
};

class Session
{
	private:

		std::string 				id;
		std::map<string, string>	_cookies;

		std::pair<string, string>	_getCookieInfo(std::string str, bool &ref);

	public:

		void	insertCookieFromString(std::string str);

		Session(); 						//Default constructor
		~Session();						//Destructor
};

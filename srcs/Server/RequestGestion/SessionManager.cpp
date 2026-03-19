#include "SessionManager.hpp"

Session & SessionManager::accessSession(const std::string &id, unsigned int server)
{
	if (server > _serverCount)
		throw (ExceptionRuntime("Session manager tried to access a non existing server"));

	sessions_it it 	= _sessions[server].begin();
	sessions_it	ite	= _sessions[server].end();

	// No session cookie
	if (id.empty())
	{
		for (; it != ite; it++)
		{
			if (_idCount == __INT_MAX__)
				throw (ExceptionRuntime("idCount reached int max (session manager)"));
			if (it->getId() == intToString(_idCount))
			{
				_idCount++;
				it = _sessions[server].begin();
				std::cout << BLUE BOLD << "found corresponding session in server " << server << std::endl;
			}
		}

		Session new_session(intToString(_idCount));
		std::cout << BLUE BOLD << "new session created in server " << server << std::endl;

		_idCount++;

		_sessions[server].push_back(new_session);
		return (_sessions[server].back());
	}

	// Looking for existing session
	
	for (; it != ite; it++)
	{
		if (it->getId() == id)
		{
			return (*it);
		}
	}

	// Didn't find existing session, inserting empty with same id
	_sessions[server].push_back(Session(intToString(_idCount)));
	_idCount++;
	return (_sessions[server].back());
}

void		SessionManager::setServerCount(unsigned int serverCount)
{
	_serverCount = serverCount;
}

void		SessionManager::initServerSessions()
{
	for (unsigned int i = 0; i < _serverCount; i++)
	{
		_sessions.push_back(std::vector<Session>());
	}
}

SessionManager::SessionManager() : _idCount(0)
{

}

SessionManager::~SessionManager()
{

}
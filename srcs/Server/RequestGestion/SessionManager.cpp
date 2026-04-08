#include "SessionManager.hpp"

sessions_by_servor	SessionManager::_sessions = sessions_by_servor();
unsigned int		SessionManager::_serverCount = 0;
unsigned int		SessionManager::_idCount = 0;

Session	*SessionManager::_createNewSession(unsigned int serverId, unsigned int sessionId)
{
	Session *ptr = new Session(intToString(sessionId));
	// std::cerr << RED BOLD << "new session created in server " << serverId << std::endl << END;

	_idCount++;
	_sessions[serverId].push_back(ptr);
	return (ptr);
}

Session *SessionManager::accessSession(const std::string &id, unsigned int serverId)
{
	// std::cerr << GREEN ITALIC << "Looking for session from id : " << id << END << std::endl;
	if (serverId >= _serverCount)
		throw (ExceptionRuntime("Session manager tried to access a non existing server"));

	sessions_it it 	= _sessions[serverId].begin();
	sessions_it	ite	= _sessions[serverId].end();

	// No session cookie
	if (id.empty())
	{
		for (; it != ite; it++)
		{
			if (_idCount == __INT_MAX__)
				throw (ExceptionRuntime("idCount reached int max (session manager)"));

			if ((*it)->getId() == intToString(_idCount))
			{
				_idCount++;
				it = _sessions[serverId].begin();
				// std::cerr << ORANGE BOLD << "found corresponding session in server " << serverId << std::endl << END;
			}
		}
		// std::cerr << ORANGE BOLD << "No cookie found, creating new one -> " << _idCount << END << std::endl;

		return (_createNewSession(serverId, _idCount));	
	}

	// Looking for existing session
	for (; it != ite; it++)
	{
		if ((*it)->getId() == id)
		{
			// std::cerr << ORANGE BOLD << "Cookie found, using -> " << id << END << std::endl;

			return (*it);
		}
	}
	// std::cerr << ORANGE BOLD << "Cookie found with an ID but no session registered, using -> " << id << END << std::endl;

	// Didn't find existing session, inserting empty with same id
	return (_createNewSession(serverId, stringToInt(id)));
}

void		SessionManager::setServerCount(unsigned int serverCount)
{
	_serverCount = serverCount;
}

void		SessionManager::initServerSessions()
{
	for (unsigned int i = 0; i < _serverCount; i++)
	{
		_sessions.push_back(std::vector<Session *>());
	}
}

void	SessionManager::clean()
{
	for (unsigned int i = 0; i < _serverCount; i++)
	{
		sessions_it it, ite;
		it	= _sessions[i].begin();
		ite	= _sessions[i].end();
		for (; it != ite; it++)
			delete *it;
	}
}

SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}
#pragma once
#include "Session.hpp"

typedef std::vector<Session *>::iterator		sessions_it;
typedef std::vector< std::vector<Session *> >	sessions_by_servor;

class SessionManager
{
	private:

		static sessions_by_servor	_sessions;
		static unsigned int			_idCount;
		static unsigned int			_serverCount;

		static Session	*_createNewSession(unsigned int serverId, unsigned int sessionId);

		SessionManager();
		~SessionManager();

	public:
	
		static void		initServerSessions();
		static Session 	*accessSession(const std::string & id, unsigned int server);
		static void		setServerCount(unsigned int serverCount);

		static	void	clean();
};
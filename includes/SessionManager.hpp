#pragma once
#include "Session.hpp"

typedef std::vector<Session>::iterator		sessions_it;
typedef std::vector< std::vector<Session> >	sessions_by_servor;

class SessionManager
{
	private:
		sessions_by_servor		_sessions;
		unsigned int			_idCount;
		unsigned int			_serverCount;


	public:
		void		initServerSessions();
		Session &	accessSession(const std::string & id, unsigned int server);
		void		setServerCount(unsigned int serverCount);

		SessionManager();
		~SessionManager();
};
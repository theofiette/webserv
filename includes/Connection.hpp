#pragma once

# include "poll.h"
# include "../srcs/utils/memory.tpp"

// TODO - SHOULD REFACTO //////////////////////////////////////////////////////

// # include "Server.hpp"
# include "Request.hpp"
// # include <Session.hpp>

class Request;

# include "Response.hpp"
// class Session;

///////////////////////////////////////////////////////////////////////////////

enum ConnectionState {
	CS_NONE,
	CS_BUFFERING,
	CS_PARSING,
	CS_WRITING,
	CS_WAIT_CGI,
	CS_SENDING,
	CS_KEEP,
	CS_CLOSE
};

class Connection {

	private:

		int			_socketFd;
		short		_wantedEvent;

		ConnectionState		_state;

		Request				_request;
		Response			_response;
		TimeoutTracker 		_timer;

		pid_t				_cgiPid;
		int					_cgiReadFd;

		const Server &		_server;
		const unsigned int	_serverId;
		Session 			*_session;

		void	_clear();
		void	_checkTimeout();
		
		void	_startCgi();
		void	_clearCgi();

		void	_retrieveSession();

		Connection();

	public:
	
		void	process(short revent);

		bool	processBuffering();
		void	processParsing();
		void	processWriting();
		void	processCgi(short revent);
		void	processSending();

		
		// short			getEvent() const;
		int				getFd() const;
		int				getPollFd() const;
		ConnectionState	getState() const;
		string			getStateInterpreted() const;
		void			setState(ConnectionState state);
		short			getWantedEvent();

		Connection &operator=(const Connection &other);
		// Connection(const Connection &other);
		// Connection(int fd);
		Connection(int fd, const Server &server, const unsigned int serverId);
		~Connection();

};
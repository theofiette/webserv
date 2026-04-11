#pragma once

# include "poll.h"
# include "../srcs/utils/memory.tpp"

# include "Request.hpp"
# include "Cgi.hpp"

class Request;

# include "Response.hpp"

enum ConnectionState {
	CS_NONE,
	CS_BUFFERING,
	CS_PARSING,
	CS_WRITING,
	CS_SEND_CGI_BODY,
	CS_LAUNCH_CGI,
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
		Cgi					_cgi;

		const Server &		_server;
		const unsigned int	_serverId;
		Session 			*_session;

		bool	_processBuffering();
		void	_processParsing();
		void	_processWriting();
		void	_processCgi(short revent);
		void	_processSending();


		void	_clear();
		void	_checkTimeout();
		
		void	_startCgi();
		void	_clearCgi();

		void	_retrieveSession();

		Connection();

	public:
	
		void			process(short revent);
		int				getFd() const;
		int				getPollFd() const;
		ConnectionState	getState() const;
		string			getStateInterpreted() const;
		void			setState(ConnectionState state);
		short			getWantedEvent();

		Connection &operator=(const Connection &other);
		Connection(int fd, const Server &server, const unsigned int serverId);
		~Connection();
};
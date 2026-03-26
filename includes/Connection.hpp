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
	CS_PARSING,
	CS_PARSED,
	CS_WRITING,
	CS_WRITTEN,
	CS_SENDING,
	CS_KEEP,
	CS_CLOSE
};

class Connection {

	private:

		int			_fd;

		ConnectionState			_state;
		// HttpRequest			_request;
		// HttpResponse			_response;
		// HttpParser			_parser; <- could be static

		Request				_request;
		Response			_response;

		const Server &		_server;
		// Session				_session;

		void	_clear();

		Connection();

	public:
	
		void	process(short revent);
		void	processTmp(short revent);
		
		// short			getEvent() const;
		int				getFd() const;
		ConnectionState	getState() const;
		void			setState(ConnectionState state);

		Connection &operator=(const Connection &other);
		// Connection(const Connection &other);
		// Connection(int fd);
		Connection(int fd, const Server &tmp_req_serv, unsigned int tmp_req_index);
		~Connection();

};
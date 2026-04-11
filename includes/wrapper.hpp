#pragma once

#include "WebServer.hpp"
#include "exception.hpp"

int		Socket(int domain, int type, int protocol);
void	Fcntl(int fd, int cmd, int opt);
void	Getaddrinfo(const std::string name, const std::string service, 
	const addrinfo *req, addrinfo **pai);
void	Setsockopt(int fd, int level, int optname);
void	Bind(int fd, const sockaddr *addr, socklen_t len);
void	Listen(int fd, int n);
void	Poll(pollfd *pollfds, nfds_t socketCount, int timeout);
void	safeClose(int &fd);
void	closePipe(int pipefd[2]);

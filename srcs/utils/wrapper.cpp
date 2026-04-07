#include "wrapper.hpp"

int	Socket(int domain, int type, int protocol)
{
	int fd;
	
	fd = socket(domain, type, protocol);
	if (fd == -1)
		throw (ExceptionBadInit(strerror(errno)));
	return (fd);
}

void	Fcntl(int fd, int cmd, int opt)
{
	int errCheck;

	errCheck = fcntl(fd, cmd, opt);
	if (errCheck == -1)
		throw (ExceptionBadInit(strerror(errno)));
}

void	Getaddrinfo(const std::string name, const std::string service,
	const addrinfo *req, addrinfo **pai)
{
	int 		errCheck;
	const char	*name_;
	const char	*service_;

	// std::cout << name << " | " << service << "\n";

	name_ = name.c_str();
	service_ = service.c_str();
	if (!name_ || !name_[0] || !service_ || !service_[0])
		throw (ExceptionBadInit("Invalid values for getaddrinfo"));
	errCheck = getaddrinfo(name_, service_, req, pai);
	if (errCheck != 0)
		throw (ExceptionBadInit(gai_strerror(errCheck)));
}

void	Setsockopt(int fd, int level, int optname)
{
	int			errCheck;
	int			optval = 1;
	socklen_t	optlen = sizeof(int);

	errCheck = setsockopt(fd, level, optname, &optval, optlen);
	if (errCheck == -1)
		throw (ExceptionBadInit(strerror(errno)));
}

void	Bind(int fd, const sockaddr *addr, socklen_t len)
{
	int errCheck;
	
	errCheck = bind(fd, addr, len);
	if (errCheck == -1)
		throw (ExceptionBadInit(strerror(errno)));
}

void	Listen(int fd, int n)
{
	int	errCheck;

	errCheck = listen(fd, n);
	if (errCheck == -1)
		throw(ExceptionBadInit(strerror(errno)));
}

void	Poll(pollfd *pollfds, nfds_t socketCount, int timeout)
{
	int errCheck;

	errCheck = poll(pollfds, socketCount, timeout);
	if (errCheck == -1)
		throw(ExceptionBadInit(strerror(errno)));
}

void	safeClose(int &fd)
{
	if (fd >= 0)
	{
		close(fd);
		fd = -1;
	}
}

void	closePipe(int pipefd[2])
{
	safeClose(pipefd[0]);
	safeClose(pipefd[1]);
}

// int Send(int &fd, std::string const& chunckedStr)
// {
// 	int bytesSent = send(fd, chunckedStr.c_str(), chunckedStr.size(), 0);
// 	if (bytesSent == -1)
// 		throw (ExceptionWriteError());
// 	return (bytesSent);
// }
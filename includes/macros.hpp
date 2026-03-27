#pragma once

# define GREEN "\033[0;32m"
# define RED "\033[1;31m"
# define BLUE "\033[0;36m"
# define YELLOW "\033[0;33m"
# define MAGENTA "\033[0;35m"
# define ORANGE "\033[38;5;208m"
# define ITALIC "\033[3m"
# define BOLD "\033[1m"
# define BG_RED "\033[41m"
# define BG_GREEN "\033[42m"
# define END "\033[0m"

typedef std::string string;

# define	SESSION_PREFIXE	"session_"
# define	COOKIE_DURATION	"max-age=424242;"

#define BACKLOG_SIZE	1024

# define CRLF_SIZE	4
# define CRLFCRLF "\r\n\r\n"
# define READ_SIZE	4096
# define SEND_SIZE	4096

#define LOCALHOST	"8080"
#define MAX_CLIENTS	1024
#define TIMEOUT		(3 * 1000)

#define CONFIG_RULE	4

#define MIME_SIZE 7

#define	METHODS_COUNT	4
enum	E_METHODS
{
	HEAD,
	GET,
	POST,
	DELETE
};

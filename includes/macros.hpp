#pragma once

typedef std::string string;

# define READ_SIZE	4096
# define SEND_SIZE	4096

#define LOCALHOST	"8080"
#define MAX_CLIENTS	1024
#define TIMEOUT		(60 * 1000)

#define CONFIG_RULE	4

#define	METHODS_COUNT	3
#define MIME_SIZE 6
enum	E_METHODS
{
	GET,
	POST,
	DELETE
};

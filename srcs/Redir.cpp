
#include "Redir.hpp"

# define	REDIR_DF_VALID	false
# define	REDIR_DF_ERR	301
# define	REDIR_DF_PATH	"html/errors/default_error.html"

void	Redir::display() const
{
	if (_valid == false)
	{
		std::cout << "not valid\n";
		return ;
	}
	std::cout << "valid : " << _errorCode << " | " << _redirPath << std::endl;
}

// CONSTRUCTORS

Redir::Redir() : 
_valid(REDIR_DF_VALID), _errorCode(REDIR_DF_ERR), _redirPath(REDIR_DF_PATH)
{
	(void)_valid, (void)_errorCode; //TODO delete
	std::cout
		<< "Default constructor called for class Redir" << std::endl;
}

// DESTRUCTOR

Redir::~Redir()
{
	std::cout
		<< "Destructor called for class Redir" << std::endl;
}

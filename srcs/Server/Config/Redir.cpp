#include "Redir.hpp"
#include <iostream>

# define	REDIR_DF_VALID	false
# define	REDIR_DF_ERR	301
# define	REDIR_DF_PATH	"html/errors/default_error.html"

bool	Redir::_df_valid		= REDIR_DF_VALID;
int		Redir::_df_errorCode	= REDIR_DF_ERR;
string	Redir::_df_redirPath	= REDIR_DF_PATH;

void	Redir::setValid(bool value)
{
	_valid = value;
}

void	Redir::setErrorCode(int value)
{
	_errorCode = value;
}

void	Redir::setRedirPath(string value)
{
	_redirPath = value;
}

bool	Redir::getValid()
{
	return (_valid);
}

int		Redir::getErrorCode()
{
	return (_errorCode);
}

string	Redir::getRedirPath()
{
	return (_redirPath);
}

void	Redir::set_df_errorCode(int value)
{
	Redir::_df_errorCode = value;
}

void	Redir::set_df_redirPath(string value)
{
	_df_redirPath = value;
}
void	Redir::set_df_valid(bool value)
{
	_df_valid = value;
}

void	Redir::clear()
{
	_valid = _df_valid;
	_errorCode = _df_errorCode;
	_redirPath = _df_redirPath;
}

void	Redir::display() const
{
	return ;
}

Redir::Redir() : 
_valid(_df_valid), _errorCode(_df_errorCode), _redirPath(_df_redirPath)
{
	// std::cout
	// 	<< "Default constructor called for class Redir" << std::endl;
}

Redir::~Redir()
{
	// std::cout
	// 	<< "Destructor called for class Redir" << std::endl;
}

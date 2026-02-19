# pragma once

# include "webserv.hpp"

class Redir
{
	private:

		bool	_valid;
		int		_errorCode;
		string	_redirPath;

	public:

		void	display() const;

	// Orthodox Canonical Form
		Redir(); 						//Default constructor
		~Redir();						//Destructor
};
		

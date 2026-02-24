# pragma once

# include <string>

class Redir
{
	private:

		bool			_valid;
		int				_errorCode;
		std::string		_redirPath;

	public:

		void	display() const;

	// Orthodox Canonical Form
		Redir(); 						//Default constructor
		~Redir();						//Destructor
};


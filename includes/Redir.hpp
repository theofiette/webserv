# pragma once

# include <string>

typedef std::string string;

class Redir
{
	private:

		static	bool	_df_valid;
		static	int		_df_errorCode;
		static	string	_df_redirPath;

		bool	_valid;
		int		_errorCode;
		string	_redirPath;

	public:

		void		display() const;
		void		clear();

		static void	set_df_errorCode(int value);
		static void	set_df_redirPath(string value);
		static void	set_df_valid(bool value);

		bool		getValid();
		int			getErrorCode();
		string		getRedirPath();

		void		setValid(bool value);
		void		setErrorCode(int value);
		void		setRedirPath(string value);

	// Orthodox Canonical Form
		Redir(); 						//Default constructor
		~Redir();						//Destructor
};


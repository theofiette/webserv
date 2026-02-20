# pragma once

# include "webserv.hpp"

# include <vector>

# include "Redir.hpp"

class Location
{
	private:

			std::vector<bool>	_methods;
			std::string			_root;
			std::string			_index;
			bool				_autoIndex;
			Redir				_redir;

	public:

		void	display() const;

		// Orthodox Canonical Form
			Location(); 						//Default constructor
			~Location();						//Destructor
};
		

#pragma once

#include "Redir.hpp"
#include "macros.hpp"
#include <vector>
#include <map>
#include <iostream>

class Location
{
	private:

			std::vector<bool>	_methods;
			std::map<int, std::string> _errMap;
			std::string			_root;
			std::string			_index;
			bool				_autoIndex;
			Redir				_redir;

	public:

		void	display() const;

		// Orthodox Canonical Form
		Location(); 						//Default constructor
		~Location();						//Destructor

		std::string 				getRoot() const;
		std::map<int, std::string>&	getErrMap();
		std::string 				getIndex() const;
		const std::vector<bool>&	getMethods() const;
		Redir						getRedir() const;
		
		void setRoot(const std::string &root);
		void setErrMap();
		void setIndex(const std::string &index);
};


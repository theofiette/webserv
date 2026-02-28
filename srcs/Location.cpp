
#include "Location.hpp"

# define LOC_DEF_AIDX	false
# define LOC_DF_METHOD	true
# define LOC_DF_ROOT	"html"
# define LOC_DF_IDX		"index.html"
# define LOC_DF_AIDX	false

bool	Location::_df_method	= LOC_DF_METHOD;
string	Location::_df_root		= LOC_DF_ROOT;
string	Location::_df_index		= LOC_DF_IDX;
bool	Location::_df_autoIndex	= LOC_DF_AIDX;

bool	Location::registerCgi(std::string cgi)
{
	std::vector<std::string>::iterator it, ite;

	it	= _cgi.begin();
	ite	= _cgi.end();
	if (cgi[0] != '.')
		return (false);
	for (; it != ite; it++)
	{
		if (*it == cgi)
			return (true);
	}
	_cgi.push_back(cgi);
	return (true);
}

void	Location::clear()
{
	_root = _df_root;
	_index = _df_index;
	_autoIndex = _df_autoIndex;
	_cgi = std::vector<std::string>();
	_methods = std::vector<bool>();
	for (int i = 0; i < METHODS_COUNT; i++)
		_methods.push_back(_df_method);
	_redir.clear();
}

Redir & Location::getRedir()
{
	return (_redir);
}

//TODO int check ?
void	Location::setMethod(bool value, int index)
{
	_methods[index] = value;
}

void	Location::setRoot(string value)
{
	_root = value;
}

void	Location::setIndex(string value)
{
	_index = value;
}

void	Location::setAutoIndex(bool value)
{
	_autoIndex = value;
}


void	Location::set_df_method(bool value)
{
	Location::_df_method = value;
}

void	Location::set_df_root(string value)
{
	Location::_df_root = value;
}

void	Location::set_df_index(string value)
{
	Location::_df_index = value;
}

void	Location::set_df_autoIndex(bool value)
{
	Location::_df_autoIndex = value;
}

void	Location::display() const
{
	std::cout << "methods : G | P | D \n          ";
	for (int i = 0; i < METHODS_COUNT; i++)
	{
		std::cout << std::noboolalpha << _methods[i];
		if (i != METHODS_COUNT - 1)
			std::cout << " | ";
	}
	std::cout << "\n";
	std::cout << "root is " << _root << "\n";
		std::cout << "index is " << _index << "\n";
		std::cout << "autoindex option is set as: " << std::boolalpha << _autoIndex << "\n";
		std::cout << "redir: ";
		_redir.display();
	std::cout << std::endl;
}

// CONSTRUCTORS

Location::Location() :
_root(_df_root), _index(_df_index), _autoIndex(_df_autoIndex)
{
	for (int i = 0; i < METHODS_COUNT; i++)
		_methods.push_back(_df_method);
	// std::cout
	// 	<< "Default constructor called for class Location" << std::endl;
}

// DESTRUCTOR

Location::~Location()
{
	// std::cout
		// << "Destructor called for class Location" << std::endl;
}

// GETTER

std::string Location::getRoot() const
{
	return (_root);
}

std::map<int, std::string>& Location::getErrors()
{
	return (_errors);
}


const std::vector<bool>&	Location::getMethods() const
{
	return (_methods);
}

std::string 		Location::getIndex() const
{
	return (_index);
}

Redir				Location::getRedir() const
{
	return (_redir);
}

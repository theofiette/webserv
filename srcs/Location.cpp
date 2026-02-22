
#include "Location.hpp"

# define LOC_DEF_METHOD	true
# define LOC_DEF_ROOT	"html"
# define LOC_DEF_IDX	"index.html"
# define LOC_DEF_IDX2	"default_error.html"

# define LOC_DEF_AIDX	false

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
_root(LOC_DEF_ROOT), _autoIndex(LOC_DEF_AIDX)
{
	static int i = 0;
	if (i == 0)
	{
		_index = LOC_DEF_IDX;
		i++;
	}
	else
		_index = LOC_DEF_IDX2;
	(void)_autoIndex; //TODO delete
	for (int i = 0; i < METHODS_COUNT; i++)
		_methods.push_back(LOC_DEF_METHOD);
	// std::cout
		// << "Default constructor called for class Location" << std::endl;
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

// SETTERS

void Location::setRoot(const std::string &root)
{
	_root = root;
}

void Location::setIndex(const std::string &index)
{
	_index = index;
}


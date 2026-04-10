
#include "Location.hpp"

# define LOC_DEF_AIDX	false
# define LOC_DF_METHOD	true
# define LOC_DF_ROOT	"html"
# define LOC_DF_IDX		"index.html"
# define LOC_DF_COOKIE	false
# define LOC_DF_UPLOAD	false
# define LOC_DF_UPDIR	"upload"
# define LOC_DF_AIDX	false

bool	Location::_df_method		= LOC_DF_METHOD;
string	Location::_df_root			= LOC_DF_ROOT;
string	Location::_df_index			= LOC_DF_IDX;
bool	Location::_df_cookie		= LOC_DF_COOKIE;
bool	Location::_df_upload		= LOC_DF_UPLOAD;
string	Location::_df_upload_dir	= LOC_DF_UPDIR;
bool	Location::_df_autoIndex		= LOC_DF_AIDX;

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
	_cookie = _df_cookie;
	_cgi = std::vector<std::string>();
	_methods = std::vector<bool>();
	_methods.reserve(METHODS_COUNT);
	for (int i = 0; i < METHODS_COUNT; i++)
		_methods.push_back(_df_method);
	_upload = _df_upload;
	_uploadDir = _df_upload_dir;
	_redir.clear();
}

Redir & Location::getRedir()
{
	return (_redir);
}

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

void	Location::setCookie(bool value)
{
	_cookie = value;
}

void	Location::setAutoIndex(bool value)
{
	_autoIndex = value;
}

void	Location::setUpload(bool value)
{
	_upload = value;
}

void	Location::setUploadDir(string value)
{
	_uploadDir = value;
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

void	Location::set_df_cookie(bool value)
{
	Location::_df_cookie = value;
}

void	Location::set_df_autoIndex(bool value)
{
	Location::_df_autoIndex = value;
}

void	Location::set_df_upload(bool value)
{
	Location::_df_upload = value;
}

void	Location::set_df_upload_dir(string value)
{
	Location::_df_upload_dir = value;
}

void	Location::insertError(int code, string path)
{
	std::pair<int, string> new_pair(code, path);

	if (_errors.find(code) == _errors.end())
		_errors.insert(new_pair);
	else
		_errors.find(code)->second = path;
}

//WORKING HERE

bool	Location::checkErrorsValidity()
{
	std::string	path;
	std::map<int, std::string>::const_iterator	it, ite;

	it	= _errors.begin();
	ite	= _errors.end();

	for (; it != ite; it++)
	{
		path = it->second;
		if (access(path.c_str(), R_OK))
			return (false);
	}
	return (true);
}

void	Location::display() const
{
	return ;
}

// CONSTRUCTORS

Location::Location() :
_root(_df_root), _index(_df_index), _autoIndex(_df_autoIndex),
_cookie(_df_cookie), _upload(_df_upload), _uploadDir(_df_upload_dir)
{
	_methods.reserve(METHODS_COUNT);
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

const std::string& Location::getRoot() const
{
	return (_root);
}

const std::vector<std::string>& Location::getCgi() const
{
	return (_cgi);
}

const std::map<int, std::string>& Location::getErrors() const
{
	return (_errors);
}

const std::vector<bool>&	Location::getMethods() const
{
	return (_methods);
}

const std::string&	Location::getIndex() const
{
	return (_index);
}

const Redir&	Location::getRedir() const
{
	return (_redir);
}

bool Location::isCookie() const
{
	return (_cookie);
}

bool Location::isAutoIndex() const
{
	return (_autoIndex);
}

const string&	Location::getUploadDir() const
{
	return (_uploadDir);
}

bool	Location::getUpload() const
{
	return (_upload);
}

#include "exception.hpp"

const char* ExceptionNull::what() const throw()
{
	return ("Can't dereference a null pointer.");
}
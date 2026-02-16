#include "exception.hpp"

const char* ExceptionNull::what() const throw()
{
	return ("Can't dereference a null pointer.");
}

const char* ExceptionSignal::what() const throw()
{
	return ("Catched a signal.");
}

int ExceptionSignal::getSig() const throw()
{
	return (_sig);
}

ExceptionSignal::ExceptionSignal(int sig) : _sig(sig)
{}
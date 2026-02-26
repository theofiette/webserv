#include "exception.hpp"

// Signal
const char* ExceptionNull::what() const throw()
{
	return ("Can't dereference a null pointer.");
}

const char* ExceptionBadInit::what() const throw()
{
	return ("Encountered an error during init.");
}

const char* ExceptionSignal::what() const throw()
{
	return ("Caught a signal.");
}

int ExceptionSignal::getSig() const throw()
{
	return (_sig);
}

ExceptionSignal::ExceptionSignal(int sig) : _sig(sig)
{}

// Parsing
const char*	ExceptionParsingError::what() const throw()
{
	return ("Parsing error found.\n"); // A approfondir 
}

const char*	ExceptionPathError::what() const throw()
{
	return ("Path error found.\n"); // A approfondir 
}

const char*	ExceptionMethodError::what() const throw()
{
	return ("Method not recognized.\n"); // A approfondir 
}

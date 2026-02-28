#include "exception.hpp"

// Signal
const char* ExceptionNull::what() const throw()
{
	return ("Can't dereference a null pointer.");
}

const char* ExceptionBadInit::what() const throw()
{
	if (!_precision.empty())
		return (_precision.c_str());
	return ("Encountered an error during init.");
}

ExceptionBadInit::ExceptionBadInit() throw()
{}

ExceptionBadInit::ExceptionBadInit(std::string str) throw() : _precision(str) 
{}

ExceptionBadInit::~ExceptionBadInit() throw()
{}

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

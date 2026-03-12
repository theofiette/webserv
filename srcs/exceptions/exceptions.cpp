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

const char* ExceptionRuntime::what() const throw()
{
	if (!_precision.empty())
		return (_precision.c_str());
	return ("Encountered an error during runtime.");
}

ExceptionRuntime::ExceptionRuntime() throw()
{}

ExceptionRuntime::ExceptionRuntime(std::string str) throw() : _precision(str) 
{}

ExceptionRuntime::~ExceptionRuntime() throw()
{}

ExceptionBadInit::ExceptionBadInit() throw()
{}

ExceptionBadInit::ExceptionBadInit(std::string str) throw() : _precision(str) 
{}

ExceptionBadInit::~ExceptionBadInit() throw()
{}


const char* ExceptionRequest::what() const throw()
{
	return ("Request not valid.");
}


const char* ExceptionSignal::what() const throw()
{
	return ("Caught a signal.");
}

ExceptionSignal::ExceptionSignal() throw()
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

const char*	ExceptionCgiPipeError::what() const throw()
{
	return ("Cgi: Pipe Error detected.\n"); // A approfondir 
}

const char*	ExceptionReadError::what() const throw()
{
	return ("Read Error detected.\n");
}

const char*	ExceptionWriteError::what() const throw()
{
	return ("Write Error detected.\n");
}
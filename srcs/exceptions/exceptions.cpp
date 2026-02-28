#include "exception.hpp"

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
	return ("Catched a signal.");
}

int ExceptionSignal::getSig() const throw()
{
	return (_sig);
}

ExceptionSignal::ExceptionSignal(int sig) : _sig(sig)
{}
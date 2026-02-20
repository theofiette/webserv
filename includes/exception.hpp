#pragma once

#include <iostream>

class	ExceptionNull : public std::exception
{
	private:
		const char*  what() const throw();
};


class	ExceptionBadInit : public std::exception
{
	private:
		const char*  what() const throw();
};

class	ExceptionSignal : public std::exception
{
	private:
		int	_sig;

		const char*	what() const throw();
		ExceptionSignal();
	
	public:
		ExceptionSignal(int sig);
		int	getSig() const throw();
};

class ExceptionParsingError : public std::exception
{
	private:
		const char*	what() const throw();
};
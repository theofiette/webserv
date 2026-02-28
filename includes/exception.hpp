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
		
		std::string	_precision;

		const char*  what() const throw();

	public:
		ExceptionBadInit()throw();
		ExceptionBadInit(std::string precision) throw();
		~ExceptionBadInit() throw();
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
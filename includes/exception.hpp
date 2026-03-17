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

class	ExceptionRuntime : public std::exception
{
	private:
		std::string _precision;
		const char*  what() const throw();

	public:
		ExceptionRuntime()throw();
		ExceptionRuntime(std::string precision) throw();
		~ExceptionRuntime() throw();
};

class	ExceptionSignal : public std::exception
{
	private:
		const char*	what() const throw();
	
	public:
		ExceptionSignal() throw();
};

class ExceptionParsingError : public std::exception
{
	public:
		const char*	what() const throw();
};

class ExceptionPathError : public std::exception
{
	private:
		const char*	what() const throw();
};

class ExceptionMethodError : public std::exception
{
	private:
		const char*	what() const throw();
};

class ExceptionCgiPipeError : public std::exception
{
	private:
		const char*	what() const throw();
};

class ExceptionRequest : public std::exception
{
	private:
		const char* what() const throw();
};

class ExceptionReadError : public std::exception
{
	private:
		const char* what() const throw();
};

class ExceptionWriteError : public std::exception
{
	private:
		const char* what() const throw();
};


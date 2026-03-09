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

// class	ExceptionMissingData : public std::exception
// {
// 	private:
// 		std::string _precision;
// 		const char*  what() const throw();

// 	public:
// 		ExceptionMissingData()throw();
// 		ExceptionMissingData(std::string precision) throw();
// 		~ExceptionMissingData() throw();
// };

class	ExceptionSignal : public std::exception
{
	private:
		const char*	what() const throw();
	
	public:
		ExceptionSignal() throw();
};

class ExceptionParsingError : public std::exception
{
	private:
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


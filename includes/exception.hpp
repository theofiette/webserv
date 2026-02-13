#pragma once

#include <iostream>

class	ExceptionNull : public std::exception
{
	const char*  what() const throw();
};
#pragma once

#include <unistd.h>
#include "exception.hpp"

template< typename T>
void	t_bzero(T *ptr, const size_t size)
{
	char *ptr_ref;
	
	ptr_ref = reinterpret_cast<char *>(ptr);

	if (!ptr_ref && !size)
		throw ExceptionNull();

	for (size_t i = 0; i < size; i++)
		ptr_ref[i] = 0;
}
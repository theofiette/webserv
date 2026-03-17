#include <stdexcept>
#include <sstream>

#include "safemath.hpp"

unsigned int	safeAddition(int a, int b)
{
	if ((a > 0 && b > __INT_MAX__ - a) || (a < 0 && b < (-__INT_MAX__ - 1) + a))
		throw (std::out_of_range(""));
	return (a + b);
}

std::string intToString(int num)
{
	std::stringstream ss;
    ss << num;
    return (ss.str());
}

# include "format.hpp"

void formatStopAt(std::string &line, char c, bool included)
{
	unsigned long length;

	length = line.find(c);
	if (length != std::string::npos)
		line = line.substr(0, line.find(c) + included);
}

void formatStopAt(std::string &line, char c)
{
	unsigned long length;

	length = line.find(c);
	if (length != std::string::npos)
		line = line.substr(0, line.find(c));
}

void formatAddSpacesAround(std::string &line, char c)
{
	std::string::iterator	it = line.begin();
	std::string				newline;
	while (*it)
	{
		if (*it == c)
		{
			newline += ' ';
			newline += *it;
			newline += ' ';
		}
		else
			newline += *it;
		it++;
	}
	line = newline;
}

void	formatRemoveConsecutiveSpaces(std::string &line)
{
	std::istringstream	stream(line);
	std::string			word;
	std::string			newline;

	while (stream >> word)
	{
		if(!newline.empty())
			newline += ' ';
		newline += word;
	}
	line = newline;
}

#include "UploadHandler.hpp"

#include "Request.hpp"

#include <sys/stat.h>
#include <fstream>

UploadHandler::UploadHandler(const Request &request, const std::string &uploadDirectory) :
_request(request), _uploadDirectory(uploadDirectory), _statusCode(400) {}

UploadHandler::~UploadHandler()  {}

std::string	UploadHandler::_extractFilename()
{
	std::map<std::string, std::string>::const_iterator it = _request.getMap().find("Filename: ");
	if (it == _request.getMap().end())
	{
		std::cout << "HERE10\n";
		return ("");
	}
	std::string filename = it->second.substr((*it).second.size() + 11);
	std::cout << ORANGE << "FILENAME IS " << filename << std::endl << END;
	return (filename);
}

bool	UploadHandler::_writeFile(const std::string &, const std::string &) const
{
	// std::ofstream outfile(filename.c_str());
	// if (!outfile.is_open())
	// 	;
	// outfile.write()
	;
	return (true);
}

int	UploadHandler::processUpload()
{
	const std::string filename = _extractFilename();
	if (filename.empty())
	{
		_statusCode = 400;
		return (_statusCode);
	}
	_statusCode = 201;
	return (_statusCode);
}

// int UploadHandler::getUploadStatus() const
// {
// 	return (_statusCode);
// }

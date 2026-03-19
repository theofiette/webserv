#pragma once

#include <string>

class Request;

class UploadHandler
{
	private:
		const Request	&_request;
		std::string		_uploadDirectory;
		int				_statusCode;

		std::string	_extractFilename();
		bool		_writeFile(const std::string &, const std::string &) const;
		
	public:
		UploadHandler(const Request &, const std::string &);
		~UploadHandler();
		
		int	processUpload();
};


#pragma once

#include <string>

class Request;

class UploadHandler
{
	private:
		const Request	&_request;
		std::string		_uploadDirectory;
		int				_statusCode;
		std::string		_boundary;
		std::string		_subHeader;

		std::string	_extractBoundary(const std::string &);
		std::string	_extractFilename() const;
		bool		_writeFile(const std::string &, const std::string &) const;
		
	public:
		UploadHandler(const Request &, const std::string &);
		~UploadHandler();

		static bool	_isMultipart(const std::string &);
		
		int	processUpload();
};


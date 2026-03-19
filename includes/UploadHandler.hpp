#pragma once

#include <string>

class Request;

struct UploadResult
{
	int		statusCode;

	UploadResult();
};

class UploadHandler
{
	private:
		const Request	&_request;
		std::string		_uploadDirectory;

		std::string	_extractFilename(const std::string &) const;
		bool		_writeFile(const std::string &, const std::string &) const;

	public:
		UploadHandler(const Request &, const std::string &);
		~UploadHandler();

		UploadResult	processUpload() const;
};


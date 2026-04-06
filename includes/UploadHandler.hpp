#pragma once

#include <string>
#include <vector>

class Request;

class UploadHandler
{
	private:
		const Request	&_request;
		std::string		_uploadDirectory;
		int				_statusCode;
		std::string		_boundary;

		std::string			_extractBoundary(const std::string &);
		std::string			_extractFilename() const;
		std::string 		_finalizePath(const std::string);
		bool				_writeFile(const std::string &, std::vector<char>&);
		std::vector<char>	_trimBody(const std::string&);
		
	public:
		UploadHandler(const Request &, const std::string &);
		~UploadHandler();

		static bool	_isMultipart(const std::string &);
		
		int			processUpload();

		const std::string findCorrespondingValueFromKey(const std::string&);
};


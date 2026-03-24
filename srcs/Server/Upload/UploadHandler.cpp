#include "UploadHandler.hpp"

#include "Request.hpp"
#include "Response.hpp"

#include <sys/stat.h>
#include <fstream>

UploadHandler::UploadHandler(const Request &request, const std::string &uploadDirectory) :
_request(request), _uploadDirectory(uploadDirectory), _statusCode(400) {}

UploadHandler::~UploadHandler()  {}

bool	UploadHandler::_isMultipart(const std::string &contentType)
{
	return (contentType.find("multipart/form-data") != std::string::npos);
}

std::string	UploadHandler::_extractBoundary(const std::string &contentType)
{
	size_t pos = contentType.find("boundary=");

	if (pos == std::string::npos)
		return ("");
	pos += 9;
	if (pos >= contentType.size())
		return ("");
	return (contentType.substr(pos));
}

std::string	UploadHandler::_extractFilename() const
{
	const std::vector<char> &bodyVec = _request.getBody();
	if (bodyVec.empty())
		return ("");

	std::string body(bodyVec.begin(), bodyVec.end());
	size_t dispositionPos = body.find("Content-Disposition:");
	if (dispositionPos == std::string::npos)
		return ("");

	size_t LineEnd = body.find("\r\n", dispositionPos);
	if (LineEnd == std::string::npos)
		return ("");

	std::string dispositionLine = body.substr(dispositionPos, LineEnd - dispositionPos);
	size_t filenamePos = dispositionLine.find("filename=\"");
	if (filenamePos == std::string::npos)
		return ("");
	filenamePos += 10;
	size_t filenameEnd = dispositionLine.find('"', filenamePos);
	if (filenameEnd == std::string::npos)
		return ("");

	std::string filename = dispositionLine.substr(filenamePos, filenameEnd - filenamePos);
	if (filename.empty())
		return ("");

	return (filename);
}

bool	UploadHandler::_writeFile(const std::string & filename, const std::string &content) const
{
	(void)filename; 
	(void)content;
	// std::ofstream outfile(filename.c_str());
	// if (!outfile.is_open())
	// 	;
	// outfile.write()
	;
	return (true);
}

// ------geckoformboundaryec4d5957babb5756bf6c14779693ebba
// Content-Disposition: form-data; name="file"; filename="nsmail.jpg"
// Content-Type: image/jpeg

int	UploadHandler::processUpload()
{
	const std::string filename = _extractFilename();
	std::map<std::string, std::string>::const_iterator contentType = _request.getMap().find("Content-Type");
	if (contentType == _request.getMap().end())
	{
		_statusCode = BAD_REQUEST;
		return (_statusCode);
	}
	const std::string _boundary = _extractBoundary(contentType->second);

	if (filename.empty())
	{
		_statusCode = BAD_REQUEST; // 400 ?
		return (_statusCode);
	}
	_statusCode = CREATED;
	return (_statusCode);
}

// int UploadHandler::getUploadStatus() const
// {
// 	return (_statusCode);
// }

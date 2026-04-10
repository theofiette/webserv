#include "UploadHandler.hpp"

#include "Request.hpp"
#include "Response.hpp"

#include <sys/stat.h>
#include <algorithm>
#include <fstream>

UploadHandler::UploadHandler(const Request &request, const std::string &uploadDirectory) :
_request(request), _uploadDirectory(uploadDirectory), _statusCode(CREATED) {}

UploadHandler::~UploadHandler() {}

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

std::string	UploadHandler::_extractFilename() const // faire juste size_t CursorStart, CursorEnd, et reutilsier les memes? 
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

	size_t pos = 0;
	while ((pos = (filename.find(".."))) != string::npos)
		filename.erase(pos, 2);
	while ((pos = (filename.find("//"))) != string::npos)
		filename.erase(pos, 1);
	return (filename);
}

bool	UploadHandler::_writeFile(const std::string &filename, std::vector<char> &content)
{
	std::ofstream outfile(filename.c_str(), std::ios::binary);
	if (!outfile.is_open())
	{
		_statusCode = INTERNAL_SERVER_ERROR;
		return (false);
	}
	std::string bodyStr;
	bodyStr.reserve(content.size());
	for (std::vector<char>::iterator it = content.begin(); it != content.end(); it++)
		bodyStr += *it;
	outfile.write(bodyStr.c_str(), content.size());
	return (true);
}

const std::string UploadHandler::findCorrespondingValueFromKey(const string &Key)
{
	mc_ss_it it = _request.getMap().find(Key);
	if (it == _request.getMap().end())
	{
		_statusCode = BAD_REQUEST;
		return ("");
	}
	return (it->second);
}


// ------geckoformboundaryec4d5957babb5756bf6c14779693ebba
// Content-Disposition: form-data; name="file"; filename="nsmail.jpg"
// Content-Type: image/jpeg

std::vector<char> UploadHandler::_trimBody(const std::string &boundary)
{
	std::vector<char> body = _request.getBody();
	std::string crlf = CRLFCRLF;

	std::vector<char>::iterator subHeaderEnd = std::search(body.begin(), body.end(), crlf.begin(), crlf.end());
	if (subHeaderEnd == body.end())
	{
		_statusCode = BAD_REQUEST;
		return (std::vector<char>(0));
	}
	size_t offset = std::distance(body.begin(), subHeaderEnd) + CRLF_SIZE;
	body.erase(body.begin(), body.begin() + offset);

	std::vector<char>::iterator lastBoundaryPos = std::search(body.begin(), body.end(), boundary.begin(), boundary.end());
	if (lastBoundaryPos == body.end()){
		_statusCode = BAD_REQUEST;
		return (std::vector<char>(0));
	}
	body.erase(lastBoundaryPos - 3, body.end());
	return (body);
}

std::string UploadHandler::_finalizePath(const std::string filename)
{
	if (_uploadDirectory[_uploadDirectory.size() - 1] == '/')
		return (_uploadDirectory + filename);
	return (_uploadDirectory + "/" + filename);
}

int	UploadHandler::processUpload()
{
	const std::string filename = _extractFilename();
	
	const string contentType = findCorrespondingValueFromKey("Content-Type");
	if (_statusCode == BAD_REQUEST)
		return (_statusCode);

	const std::string boundary = _extractBoundary(contentType);

	if (filename.empty())
	{
		_statusCode = BAD_REQUEST; // 400 ?
		return (_statusCode);
	}
	// std::cout << "CHECK PASSED \n\n";

	std::vector<char> fileContent = _trimBody(boundary);
	if (fileContent.empty())
	{
		_statusCode = BAD_REQUEST;
		return (_statusCode);
	}
	std::string finalPath = _finalizePath(filename);
	if (_writeFile(finalPath, fileContent) == false)
		return (_statusCode);

	_statusCode = CREATED;
	return (_statusCode);
}

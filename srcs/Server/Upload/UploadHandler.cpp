#include "UploadHandler.hpp"

#include "Request.hpp"

#include <sys/stat.h>
#include <fstream>

UploadHandler::UploadHandler(const Request &request, const std::string &uploadDirectory) :
_request(request), _uploadDirectory(uploadDirectory) {}

UploadHandler::~UploadHandler()  {}

// std::string	UploadHandler::_extractFilename(const std::string &) const
// {
// 	;
// }

// bool	UploadHandler::_writeFile(const std::string &, const std::string &) const
// {
// 	// std::ofstream outfile(filename.c_str());
// 	// if (!outfile.is_open())
// 	// 	;
// 	// outfile.write()
// 	;
// }

// UploadResult	UploadHandler::processUpload() const
// {
// 	UploadResult result;
	
// }

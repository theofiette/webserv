#include "webserv.hpp"
#include "WebServer.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Request.hpp"

#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <sys/wait.h>

const std::string Response::methods[METHODS_COUNT] = {"GET", "POST", "DELETE"};
const std::string Response::mimeTable[MIME_SIZE] = {"html", "css", "png", "ico", "gif", "plain"};

static std::map<int, std::string> initStatusMap() 
{
	std::map<int, std::string> m;

	m[200] = "OK";
	m[201] = "Created";
	m[204] = "No content";
	m[301] = "Move permanently";
	m[302] = "Found";
	m[303] = "See other";
	m[403] = "Forbidden";
	m[404] = "Not found";
	m[405] = "Not allowed";
	m[413] = "Content too large";
	
	return (m);
}

const std::map<int, std::string> Response::_statusMap = initStatusMap();

//*********************************************************************************************/
/*										CONSTRUCTORS										  */
//*********************************************************************************************/

Response::Response(const Request &request, const Server &server) : _statusCode(200),  _bytesSent(0), _headerLength(0), _contentLength(0), 
_currServ(server), _request(request)
{
	_request.displayInfos();
	if (_request.getMap().find("Method") == _request.getMap().end())
		throw (ExceptionRuntime("Invalid request for response constructor (Content Length)"));
	if (_request.getMap().find("Content-Length") != _request.getMap().end() && stringToInt(_request.getMap().at("Content-Length")) > _currServ.getMaxBody())
		_statusCode = CONTENT_TOO_LARGE;
	pickMethod();
	buildRawResponse();
	_bytesSent = 0;
}

Response::~Response(){}

//*********************************************************************************************/
/*										METHODS											  	  */
//*********************************************************************************************/

std::string Response::intToString(int num)
{
    std::stringstream ss;
    ss << num;
    return (ss.str());
}

void Response::pickMethod()
{
	static void (Response::*methods[])() = 
	{
		&Response::handleGet,
		&Response::handlePost,
		&Response::handleDelete
	};
	if (_request.getMap().find("Method") == _request.getMap().end())
		throw (ExceptionRuntime("Invalid pick method for response (Method)"));
	if (_request.getMap().at("Method") == "HEAD")
		handleGet();
	if (_request.getMap().find("Path") == _request.getMap().end())
		throw (ExceptionRuntime("Invalid pick method for response (Path)"));
	if (_request.getMap().at("Path").find(".cgi") != std::string::npos) //TODO: aller cher dans la loc
	{
		// handleCgi();
		return ;
	}
	else
	{
		for (int i = 0; i < METHODS_COUNT; i++)
			if (_request.getMap().at("Method") == Response::methods[i])
			{
				try {
					(this->*methods[i])();
					return ;
				}
				catch (ExceptionPathError &e){
					return ;
				}
			}
	}
	throw ExceptionParsingError(); // nouvelle class exception ? 
}

std::string Response::MIMEtable(std::string localPath)
{
	int i = 0;
	for (; i < 5; i++)
		if (localPath.find("." + mimeTable[i]) != std::string::npos) // TODO: securise
			break ;
	return ("text/" + mimeTable[i]);
}

//*********************************************************************************************/
/*										SETTERS											  	  */
//*********************************************************************************************/

void Response::setStatusCode()
{
	;
}

void Response::addHeader(std::string key, std::string value)
{
	_header.insert(std::pair<string, string>(key, value));
}

void Response::setErrorResponse(std::string &localPath, std::map<int, std::string> const& errMapLocation, std::string const& root)
{
	// Regarder la gestion du bloc LOCATION
	// Si pas preciser rediriger vers ERROR PAGE par defaut
	mc_is_ite errPage = errMapLocation.find(_statusCode);
	if (errPage != errMapLocation.end())
	{
		localPath = root + "/" + errPage->second;
		_statusCode = MOVED_PERMANENTLY;
	}
	else
	{
		errPage = _currServ.getErrorsServer().find(_statusCode);
		if (errPage != _currServ.getErrorsServer().end())
		{
			localPath = errPage->second;
		}
		else{
			localPath = _currServ.getErrorsServer().find(0)->second;
		}
	}
	std::cout << "LOCALPATH : " << localPath << "\n";
}
//*********************************************************************************************/
/*										BUILDERS											  */
//*********************************************************************************************/

std::map<string, Location>::const_reverse_iterator Response::findBestLocation(const std::string &uriPath)
{
	const std::map<std::string, Location>& loc = _currServ.getLocations();
	
	for (std::map<string, Location>::const_reverse_iterator it = loc.rbegin(); it != loc.rend(); it++)
	{
		if (uriPath.find(it->first) == 0)
		{
			setCurrLocation(it->second);
			return (it);
		}
	}
	return (loc.rend());
}

void Response::validateMethodAllowed(const Location &location)
{
	std::string method = _request.getMap().at("Method");
	const std::vector<bool>& acceptedMethods = location.getMethods();
	
	int methodIndex = -1;
	for (int i = 0; i < METHODS_COUNT; i++)
	{
		if (methods[i] == method)
		{
			methodIndex = i;
			break;
		}
	}
	
	if (methodIndex == -1 || !acceptedMethods[methodIndex])
	{
		std::cout << "Method not accepted\n";
		throw ExceptionPathError();
	}
}

void Response::appendIndexToDirectory(std::string &localPath, const Location &location)
{
	struct stat s;

	if (stat(localPath.c_str(), &s) != -1 && S_ISDIR(s.st_mode))
	{
		std::string index = location.getIndex();
		if (localPath[localPath.size() - 1] != '/')
			localPath += "/";
		localPath += index;
	}
}

std::string Response::buildLocalPath()
{
	std::string uriPath = _request.getMap().at("Path");
	std::map<string, Location>::const_reverse_iterator bestMatch = findBestLocation(uriPath);
	
	// validateMethodAllowed(bestMatch->second);
	
	std::string localPath = bestMatch->second.getRoot() + uriPath;
	appendIndexToDirectory(localPath, bestMatch->second);
	
	return (localPath);
}

//*********************************************************************************************/
/*										UTILS											  	  */
//*********************************************************************************************/


void	Response::insertCookieHeader(std::string cookie)
{
	_header.insert(std::pair<string, string>("Set-Cookie", cookie));
}

void Response::checkFileAccess(std::string &localPath, int requiredAccess)
{
	struct stat s;
	if (stat(localPath.c_str(), &s) == -1) {
		_statusCode = NOT_FOUND;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
	}
	else if (!S_ISREG(s.st_mode) || access(localPath.c_str(), requiredAccess) == -1) {
		_statusCode = FORBIDDEN;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
	}
}

bool Response::readFileContent(const std::string &filePath, size_t fileSize)
{
	_body.resize(fileSize);
	std::ifstream infile(filePath.c_str(), std::ios::binary);
	if (fileSize > 0)
		infile.read(&_body[0], fileSize);
	return (true);
}

void Response::parseFormData(std::map<std::string, std::string> &data)
{
	size_t lastPos = 0;

	for (size_t i = _request.getBody().find("=", 0); i != std::string::npos; i = _request.getBody().find("=", lastPos))
	{
		size_t endPos = _request.getBody().find("&", i);
		if (endPos == std::string::npos)
			endPos = _request.getBody().find("\r\n\r\n");
		std::string key = _request.getBody().substr((lastPos == 0 ? lastPos : lastPos + 1), i - (lastPos + (lastPos != 0))); // 🤮🤮🤮
		std::string val = _request.getBody().substr(i + 1, endPos - (i + 1));
		data.insert(std::make_pair(key, val));
		lastPos = endPos;
	}
}

//*********************************************************************************************/
/*										HANDLERS											  */
//*********************************************************************************************/

void Response::handleGet()
{
	std::string localPath;
	struct stat s;

	if (_statusCode == CONTENT_TOO_LARGE)
	{
		mc_is_ite errPage = _currServ.getErrorsServer().find(_statusCode);
		if (errPage != _currServ.getErrorsServer().end())
			localPath = errPage->second;
		else{
			localPath = _currServ.getErrorsServer().find(0)->second;
		}
	}
	else 
		localPath = buildLocalPath();
	checkFileAccess(localPath, R_OK);
	if ((stat(localPath.c_str(), &s) == -1))
		return ;
	readFileContent(localPath, s.st_size);
	addHeader("Content-Type", MIMEtable(localPath));
	addHeader("Content-Length", intToString(_body.length()));
}

void Response::handlePost()
{
	//TODO: RECUPER DYNAMIQUEMENT SUCCESS HTML via GLOBALSERVER ou REDIR (build full path...)
	struct stat s;
	std::string localPath = "html/success.html";
	// std::string localPath = getSuccessPath();

	if (stat(localPath.c_str(), &s) == -1)
		return ;

	std::map<std::string, std::string> formData; // TODO: utiliser cette data? Du coup on appelera parseform avec &data de la class
	parseFormData(formData);

	_statusCode = SEE_OTHER;
	addHeader("Location", "http://localhost:8080/success.html");
	addHeader("Content-Length", "0");
}

void Response::handleDelete()
{
	std::string localPath = buildLocalPath();
	struct stat s;

	if (stat(localPath.c_str(), &s) == -1)
	{
		_statusCode = NOT_FOUND;
		setErrorResponse(localPath, _currLocation.getErrors(), _currLocation.getRoot());
		return ;
	}

    checkFileAccess(localPath, W_OK);
	
	remove(localPath.c_str()); // TODO: securise
	_statusCode = NO_CONTENT;
	addHeader("Content-Length", "0");
}

void Response::handleCgi(Request const& request){
	Cgi cgi(request);

	int pipe_in[2];
	int pipe_out[2];

	if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
		throw ExceptionCgiPipeError();
	
	// write()
	pid_t pid = fork();
	if (!pid)
	{
		close(pipe_in[1]);
		close(pipe_out[0]);
	
		dup2(pipe_in[0], STDIN_FILENO); //pour lire le contenu du POST? ou directement depuis pipein[0]

		if (dup2(pipe_out[1], STDOUT_FILENO) == -1)
		{
			close(pipe_in[0]);
			close(pipe_out[1]);
			exit(1);
		}
		std::string file = "./" + (cgi.getMetaVars().find("SCRIPT_NAME")->second).substr(1);
		
		char **tmp;
		tmp = new char*[2];
		tmp[0] = strdup(file.c_str());
		tmp[1] = NULL;
		execve(tmp[0], tmp, cgi.getEnv());
		strerror(errno);
		exit(0);
	}
	else
	{
		close(pipe_in[0]);
		close(pipe_out[1]);
		if (_request.getMap().find("Method")->second == "POST")
		{
			cgi.setBodyMessage(_request.getBody()); //write in stdout
			write(pipe_in[1], cgi.getBodyMessage().c_str(), cgi.getBodyMessage().size());
		}
		close(pipe_in[1]);
		waitpid(pid, NULL, 0); //ADD TIMEOUT TO AVOID ZOMBIE PROCESS?

		char buffer[4096];
		int bytesRead;

		while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer))) > 0){
			_rawResponse += buffer;
		}
		close(pipe_out[0]);
		if (bytesRead == -1)
		{
			throw ExceptionCgiPipeError(); // vrai error
		}
	}
}

std::string Response::getChunckResp(int bytes)
{
	std::string chunckResp;
	std::cout << "RAW RESP == " << _rawResponse << "\n";

	chunckResp = _rawResponse.substr(_bytesSent, bytes);
	return (chunckResp);
}

void	Response::updateBytesSent(int bytes)
{
	_bytesSent += bytes;
}

bool	Response::isSent()
{
	return (_bytesSent >= _rawResponse.size());
}


void Response::buildRawResponse()
{
	std::map<std::string, std::string>::iterator it 		= _header.begin();

	if (_request.getMap().find("Protocol") == _request.getMap().end())
		throw (ExceptionRuntime("Invalid request for build constructor (Method)"));
	_rawResponse += _request.getMap().at("Protocol") + " ";
	_rawResponse += intToString(_statusCode) + " " + _statusMap.at(_statusCode) + "\r\n";

	for (; it != _header.end(); it++)
		_rawResponse += it->first + ": " + it->second + "\r\n";
	
	_rawResponse += "\r\n";

	_contentLength = stringToInt(extractContentLength(_rawResponse));

	_headerLength = _rawResponse.size();
	_rawResponse += _body;
	std::cout << ORANGE << _statusCode << END << std::endl;
}

//*********************************************************************************************/
/*										GETTERS											  	  */
//*********************************************************************************************/

void Response::setCurrLocation(const Location &loc){
	_currLocation = loc;
}

const std::string& Response::getRawResponse() const
{
	return (_rawResponse);
}

int Response::getBytesSent() const
{
	return (_bytesSent);
}

int Response::getHeaderLength() const
{
	return (_headerLength);
}

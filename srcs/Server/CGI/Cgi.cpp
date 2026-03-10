#include "Cgi.hpp"

Cgi::Cgi(const Request &request)
{
	std::map<std::string, std::string> rm = request.getMap();

	// _metaVariables["AUTH_TYPE"] = "";
	_metaVariables["CONTENT_LENGTH"] = (rm.find("Content-length") != rm.end() ? rm.find("Content-length")->second : "");
	_metaVariables["CONTENT_TYPE"] = (rm.find("Content-type") != rm.end() ? rm.find("Content-type")->second : "");
	_metaVariables["GATEWAY_INTERFACE"] = "CGI/1.1";
	// _metaVariables["PATH_INFO"] = "";
	// _metaVariables["PATH_TRANSLATED"] = "";
	// _metaVariables["QUERY_STRING"] = (rm.find("Path") != rm.end() ? rm.find("Path")->second : "");
	// _metaVariables["REMOTE_ADDR"] = "";
	// _metaVariables["REMOTE_HOST"] = "";
	// _metaVariables["REMOTE_IDENT"] = "";
	// _metaVariables["REMOTE_USER"] = "";
	_metaVariables["REQUEST_METHOD"] = (rm.find("Method") != rm.end() ? rm.find("Method")->second : "");
	_metaVariables["SCRIPT_NAME"] = (rm.find("Path") != rm.end() ? rm.find("Path")->second : ""); // TODO:substr
	_metaVariables["SERVER_NAME"] = request.getServer().getServerName();
	_metaVariables["SERVER_PORT"] = request.getServer().getListen();
	_metaVariables["SERVER_PROTOCOL"] = (rm.find("Protocol") != rm.end() ? rm.find("Protocol")->second : "");
	// _metaVariables["SERVER_SOFTWARE"] = "";
	_metaVariables["scheme"] = "";
	_metaVariables["protocol-var-name"] = "";
	_metaVariables["extension-var-name"] = "";
}

Cgi::Cgi()
{

}

Cgi::~Cgi()
{

}

const std::map<std::string, std::string>& Cgi::getMetaVars() const{
	return (_metaVariables);
}

char* const* Cgi::getEnv() const{
	return (_cgiEnv);
}

const std::string& Cgi::getBodyMessage() const{
	return (_bodyMessage);
}

void Cgi::setBodyMessage(const std::string &body){
	_bodyMessage = body;
}

void Cgi::setupCgiFrame()
{
	
}
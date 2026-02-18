#pragma once

#include <map>
#include <iostream>

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		std::map<std::string, std::string> _statusMap;
	public:
		void setStatusCode();
		void setBody();
		void addHeader(std::string &key, std::string &value);

		void setErrorResponse(int statusCode);

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string getResponse();

};
#pragma once

#include <map>
#include <iostream>
#include <string>
#include "exception.hpp"

class Request;

class Response
{
	private:
		int _statusCode;
		std::map<std::string, std::string> _header;
		std::string _body;
		std::map<std::string, std::string> _statusMap;

		static void (*_methods[])();
		static std::string method[3]; // TODO: pas tres scalable si on a dautres methodes a check, vector? 

	public:
		Response(Request &request);
		~Response();
		void setStatusCode();
		void setBody();
		void addHeader(std::string &key, std::string &value);

		void setErrorResponse(int statusCode);

		void pickMethod(std::string method);

		void handleGet();
		void handlePost();
		void handleDelete();

		std::string const getResponse();
};
#pragma once

#include "webserv.hpp"
#include "Redir.hpp"
#include "macros.hpp"
#include <vector>
#include <map>
#include <iostream>

class Location
{
	private:

		static	bool	_df_method;
		static	string	_df_root;
		static	string	_df_index;
		static	bool	_df_cookie;
		static	bool	_df_autoIndex;
		static	bool	_df_upload;
		static	string	_df_upload_dir;

		std::vector<bool>			_methods;
		std::vector<std::string>	_cgi;
		std::map<int, string>		_errors;
		std::string					_root;
		std::string					_index;
		bool						_autoIndex;
		bool						_cookie;
		bool						_upload;
		string						_uploadDir;
		Redir						_redir;

	public:

		bool	registerCgi(std::string cgi);
		void	insertError(int code, string path);
		bool	checkErrorsValidity();
		void	display() const;
		void	clear();

		Redir &getRedir();

		static	void	set_df_method(bool value);
		static	void	set_df_root(string value);
		static	void	set_df_index(string value);
		static	void	set_df_cookie(bool value);
		static	void	set_df_upload(bool value);
		static	void	set_df_upload_dir(string value);
		static	void	set_df_autoIndex(bool value);

		void	setMethod(bool value, int index);
		void	setRoot(string value);
		void	setIndex(string value);
		void	setCookie(bool value);
		void	setAutoIndex(bool value);
		void	setUpload(bool value);
		void	setUploadDir(string value);

		// Orthodox Canonical Form
		Location(); 						//Default constructor
		~Location();						//Destructor

		const std::string&					getRoot() const;
		const std::vector<std::string>&		getCgi() const;
		const std::map<int, std::string>&	getErrors() const;
		const std::string& 					getIndex() const;
		const std::vector<bool>&			getMethods() const;
		const Redir&						getRedir() const;
		const string&						getUploadDir() const;
		bool								getUpload() const;
		bool								isCookie() const;
		bool								isAutoIndex() const;
	};


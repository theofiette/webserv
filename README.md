# Webserv

[![C++](https://img.shields.io/badge/language-C++98-555?style=flat-square&logo=cplusplus)](https://en.cppreference.com/w/cpp)
![License](https://img.shields.io/badge/license-Educational-lightgrey?style=flat-square)

An HTTP/1.1 web server in C++98, focusing on non-blocking network programming, the HTTP protocol and process management.

The server is configured with an NGINX-style file, serves static websites, handles uploads and CGI scripts, and keeps connections alive, all from a single `poll()` loop, with no external library.

---

## Build & Run

### Requirements
- Linux
- `g++` or `clang++` (C++98), `make`
- `python3` and `bash` for the example CGI scripts

> Note: Webserv has been tested on **Fedora Linux 44** and **Ubuntu 24.04**, with g++ 13, g++ 14 and clang++ 18.

### Compilation
`make`

### Run
`./webserv <config_file>`  
ex : `./webserv config/default.conf`

Then open [http://127.0.0.1:8080](http://127.0.0.1:8080) in a browser. The demo site has a page for each feature: methods, CGI, errors, cookies, session and upload.

### Configuration Files
- `config/default.conf` : one server on port 8080 with every feature
- `config/multipleServer.conf` : several servers on several ports

---

## Features

- **Methods** : GET, POST, DELETE and HEAD
- **Static website** : index files, directory listing (autoindex), MIME types
- **Uploads** : `multipart/form-data`, with a configurable storage folder
- **CGI** : executed by file extension (`.py`, `.sh`, `.cgi`, `.out`), with the standard CGI environment variables
- **Keep-alive** : several requests on the same connection
- **Chunked requests** : `Transfer-Encoding: chunked` bodies are decoded
- **Cookies and sessions** : a session ID cookie, server-side sessions, and cookie-based themes for the demo site
- **Errors** : accurate status codes (403, 404, 405, 413, 500, 504…), with custom or default error pages
- **Redirections** : configurable per route
- **Resilience** : a stuck CGI script is stopped and answered with `504 Gateway Timeout`; the server stays available

### Configuration

The configuration file syntax is inspired by NGINX. Each `server` block listens on a port and defines routes. The parser is deliberately permissive: it accepts the file as long as the information comes in the expected order.

```nginx
server {
	listen 8080;
	name 127.0.0.1;

	client_body_max 500000;
	error_page 404 html/errors/404_error.html;

	location /cgi
	{
		root cgi;
		index my_cgi.py;
		cgi .py .sh .cgi .out;
		methods GET POST on DELETE off;
	}

	location /old
	{
		return 302 /methods.html;
	}
}
```

Per route: accepted methods, redirection, root folder, autoindex, index file, upload folder, CGI extensions and cookies.

---

## Technical Overview

### Architecture

The server is built around a **`Connection` object**: one per client, holding its request, its response, its session and its CGI. Each connection is a **state machine**:

**buffering → parsing → writing → (CGI) → sending → keep-alive or close**

This links each request to its response, and lets one connection serve several requests in a row (keep-alive).

- **Event loop** : `WebServer.cpp`, `ConnectionManager.cpp`
  - A single `poll()` watches every socket and CGI pipe, for reading and writing at the same time
  - All sockets and pipes are non-blocking; nothing is read or written without `poll()` saying it is ready
- **Connections** : `Connection.cpp`
  - State machine, timeouts, keep-alive
- **HTTP** : `Request.cpp`, `Response.cpp`, `UploadHandler.cpp`
  - Request parsing (headers, body, chunked encoding), response building, uploads
- **CGI** : `Cgi.cpp`
  - `fork` / `execve` with two non-blocking pipes. Each connection has one slot in the `poll()` set: while a CGI runs, that slot watches the CGI pipe instead of the client socket, so a slow script never blocks other clients
- **Configuration** : `ConfigParser.cpp`, `Server.cpp`, `Location.cpp`, `Redir.cpp`
  - Parsing of the configuration file and socket creation for each server
- **Sessions** : `Session.cpp`, `SessionManager.cpp`
- **Utilities** : signal handling, exceptions, system call wrappers

### Testing

- Stress-tested with `siege` during development
- 200 simultaneous clients sending 20 keep-alive requests each: 4,000 responses out of 4,000
- No memory leak and no file descriptor left open at exit (Valgrind), including after CGI runs and uploads

---

## More

### Collaboration

This project was developed by a team of three:
- **Theo Fiette** / [theofiette](https://github.com/theofiette) : re-architecture around the `Connection` object to link requests and responses and support keep-alive, `poll()` loop, configuration parsing and socket creation, sessions, signals and exceptions
- **Ilies Hadj** / [Sosonyx](https://github.com/Sosonyx) : requests and responses, non-blocking CGI, uploads, timeouts, memory and file descriptor leaks
- **Teo Chevallier** / [tchevall](https://github.com/tchevall) : first network loop, error handling and error pages, chunked transfer encoding, HTML pages, Python CGI

### Educational Context

This project was developed as part of a programming curriculum (42 Paris) and aims to demonstrate:

- Non-blocking network programming with a single event loop
- Implementation of a subset of HTTP/1.1
- Process management and inter-process communication (CGI)
- Teamwork on a shared C++ codebase

> Note: This project is for educational purposes. It is not intended for production use.

### Resources

- [RFC 9112 — HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112) and [RFC 9110 — HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 3875 — The Common Gateway Interface](https://www.rfc-editor.org/rfc/rfc3875)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [NGINX documentation](https://nginx.org/en/docs/), used as a reference for configuration and behaviour

### AI Usage

This README was written with the help of Claude (Anthropic).

Webserv is a team project, and each member managed their own use of AI. On my side (Theo), I used AI to help with debugging, but did not rely on it to write my code.

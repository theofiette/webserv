#!/bin/bash

if [ "$REQUEST_METHOD" = "GET" ]; then
	BODY="<html>
		<head>
			<link rel="stylesheet" href="css/style/style.css">
		</head>
		<body>
			<div class="container">
				<h1>
					Server is : $SERVER_NAME<br>
					J'ecoute sur le port $SERVER_PORT via le $SERVER_PROTOCOL<br>
					Je suis un CGI et je fonctionne correctement avec <p> $REQUEST_METHOD <p>
				</h1>
			</div>
		</body>
	</html>"

	CONTENT_LENGTH=${#BODY}
	echo -ne "$SERVER_PROTOCOL 200 OK\r\n"
	echo -ne "Content-Type: text/html\r\n"
	echo -ne "Content-Length: $CONTENT_LENGTH\r\n"
	echo -ne "\r\n"
	echo -n "$BODY"
elif [ "$REQUEST_METHOD" = "POST" ]; then

	CONTENT_LENGTH=${#BODY}
	INPUT=$(cat)


	BODY="<html>
		<head>
			<link rel="stylesheet" href="css/style/style.css">
		</head>
		<body>
			<div class="container">
				<h1>
					Server is : $SERVER_NAME<br>
					J'ecoute sur le port $SERVER_PORT via le $SERVER_PROTOCOL<br>
					Je suis un CGI et je fonctionne correctement avec <p> $REQUEST_METHOD </p>
					Form data: $INPUT
				</h1>
			</div>
		</body>
	</html>"

	CONTENT_LENGTH=${#BODY}
	echo -ne "$SERVER_PROTOCOL 200 OK\r\n"
	echo -ne "Content-Type: text/html\r\n"
	echo -ne "Set-Cookie: theo=leplusbo\r\n"
	echo -ne "Content-Length: $CONTENT_LENGTH\r\n"
	echo -ne "\r\n"
	echo -n "$BODY"
fi

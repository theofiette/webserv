#!/bin/bash

if [ "$REQUEST_METHOD" = "GET" ]; then
	BODY="<html>
		<head>
			<style>
				*{
					background: linear-gradient(135deg, #666EB5 0%, #AA6E6E 100%);
					font-size:30px;
					font-family:'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
				}
				.container{
					margin: auto;
					text-align:center;
					justify-content:center;
					padding: 200px;
					border-radius: 10px;
					box-shadow: 0 10px 25px #303030;
					max-width: 600px;
					background-color: #fefefe;
				}
				body{
					min-height: 100vh;
					display: flex;
					align-items: center;
				}
				p{
					text-decoration: underline;
					}
			</style>
		</head>
		<body>
			<div class="container">
				<h1>
					Server is : $SERVER_NAME<br>
					J'ecoute sur le port $SERVER_PORT via le $SERVER_PROTOCOL<br>
					Je suis un CGI et je fonctionne correctement avec <p> $REQUEST_METHOD <p>
					Query-string: $QUERY_STRING
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
elif [ "$REQUEST_METHOD" = "POST" ]; then

	CONTENT_LENGTH=${#BODY}
	INPUT=$(cat)


	BODY="<html>
		<head>
			<style>
				*{
					background: linear-gradient(135deg, #AA6E6E 0%, #666EB5 100%);
					font-size:30px;
					font-family:'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif;
				}
				.container{
					margin: auto;
					text-align:center;
					justify-content:center;
					padding: 200px;
					border-radius: 10px;
					box-shadow: 0 10px 25px #303030;
					max-width: 600px;
					background-color: #fefefe;
				}
				body{
					min-height: 100vh;
					display: flex;
					align-items: center;
				}
				p{
					text-decoration: underline;
					}
			</style>
		</head>
		<body>
			<div class="container">
				<h1>
					Server is : $SERVER_NAME<br>
					J'ecoute sur le port $SERVER_PORT via le $SERVER_PROTOCOL<br>
					Je suis un CGI et je fonctionne correctement avec <p> $REQUEST_METHOD </p>
					Form data: $INPUT
					Query-string: $QUERY_STRING
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
fi

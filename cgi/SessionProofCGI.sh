#!/bin/bash

if [ "$NAME" = "" ]; then
	TXT="Hello anonymous person.";
else
	TXT="Hello $NAME !! <br>Good to see you back";
fi

BODY="<html>
		<head>
			<link rel="stylesheet" href="/style/">
		</head>
		<body>
			<div class="container">
				<h1>
					$TXT
				</h1>
			</div>

		<div>
			<div class="container"><a class="button" href="/index.html"> back</a></div>
		</div>

		</body>
	</html>"

	CONTENT_LENGTH=${#BODY}
	echo -ne "$SERVER_PROTOCOL 200 OK\r\n"
	echo -ne "Content-Type: text/html\r\n"
	echo -ne "Content-Length: $CONTENT_LENGTH\r\n"
	echo -ne "\r\n"
	echo -n "$BODY"
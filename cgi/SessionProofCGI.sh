#!/bin/bash

if [ "$USER" = "" ]; then
	echo "GET / HTTP/1.1\r\nHello anonymous person.\r\n\r\n";
else
	echo "GET / HTTP/1.1\r\nHello $USER !! Good to see you back\r\n\r\n";
fi


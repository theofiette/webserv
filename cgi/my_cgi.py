#!/usr/bin/env python3
import os
import sys
from urllib.parse import parse_qs

request_method = os.environ.get("REQUEST_METHOD", "GET")
server_name = os.environ.get("SERVER_NAME", "unknown")
server_port = os.environ.get("SERVER_PORT", "unknown")
server_protocol = os.environ.get("SERVER_PROTOCOL", "HTTP/1.1")
script_name = os.environ.get("SCRIPT_NAME", "unknown")
query_string = os.environ.get("QUERY_STRING", "unknown")

# Récupération des données POST (safe)

if request_method == "POST":
    try:
        content_length = int(os.environ.get("CONTENT_LENGTH", "0"))
    except ValueError:
        content_length = 0

    if content_length > 0:
        content_length = int(os.environ.get("CONTENT_LENGTH", 0))
        body = sys.stdin.read(content_length)
        # parse_qs retourne dict[str, list[str]], on prend le premier élément
        data = {k: v[0] for k, v in parse_qs(body).items()}


if request_method == "GET":
    body = f"""<html>
    <head>
		<link rel="stylesheet" href="css/style/style.css">
    </head>
    <body>
        <div class="container">
            <h1>
                Server is : [{server_name}]<br>
                J'ecoute sur le port [{server_port}] via le [{server_protocol}]<br>
                Je suis le cgi [{script_name}] et je fonctionne correctement avec <p>[{request_method}]</p>
                Query-string [{query_string}]
            </h1>
        </div>
    </body>
</html>"""

elif request_method == "POST":
    data_html = "".join(f"<p>{key} = {value}</p>" for key, value in data.items() if key != "password")

    body = f"""<html>
    <head>
        <link rel="stylesheet" href="css/style/style.css">
    </head>
    <body>
        <div class="container">
            <h1>
                Server is : [{server_name}]<br>
                J'ecoute sur le port [{server_port}] via le [{server_protocol}]<br>
                Je suis le cgi [{script_name}] et je fonctionne correctement avec <p>[{request_method}]</p>
                {data_html}
            </h1>
        </div>
    </body>
    </html>"""

# Headers CGI
print(f"{server_protocol} 200 OK")
print("Content-Type: text/html")
print(f"Content-Length: {len(body)}")
print()  # ligne vide obligatoire
print(body)
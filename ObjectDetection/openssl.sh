openssl req -x509 -newkey rsa:2048 -days 365 -nodes -keyout server.key -out server.crt -subj "/CN=localhost"

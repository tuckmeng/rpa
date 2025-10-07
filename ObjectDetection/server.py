import http.server
import http.server
import ssl

PORT = 8000
handler = http.server.SimpleHTTPRequestHandler
httpd = http.server.HTTPServer(('localhost', PORT), handler)

context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
context.load_cert_chain(certfile='server.crt', keyfile='server.key')

httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

print(f"Serving HTTPS on https://localhost:{PORT}")
httpd.serve_forever()

#!/usr/bin/env python3
import http.server
import socketserver
import ssl
import sys
from pathlib import Path

PORT = 5000
DIRECTORY = Path(__file__).parent / "dist"
CERT_DIR = Path(__file__).parent / "certs"


class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(DIRECTORY), **kwargs)

    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "cross-origin")

        if self.path.endswith(".wasm"):
            self.send_header("Content-Type", "application/wasm")

        super().end_headers()


def main():
    if not DIRECTORY.exists():
        print(f"Error: Build directory not found: {DIRECTORY}")
        print("Run 'npm run build' first!")
        sys.exit(1)

    cert_file = CERT_DIR / "cert.pem"
    key_file = CERT_DIR / "key.pem"

    if not cert_file.exists() or not key_file.exists():
        print(f"Error: SSL certificate not found!")
        print(f"Run './generate-cert.sh' to generate certificates")
        sys.exit(1)

    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(certfile=str(cert_file), keyfile=str(key_file))
        httpd.socket = context.wrap_socket(httpd.socket, server_side=True)

        print(f"╔══════════════════════════════════════════════════════╗")
        print(f"║  Lab2QRCode Web (HTTPS)                              ║")
        print(f"╠══════════════════════════════════════════════════════╣")
        print(f"║  🔒 HTTPS Server running at:                         ║")
        print(f"║                                                      ║")
        print(f"║     https://localhost:{PORT}                            ║")
        print(f"║     https://192.168.9.100:{PORT}                        ║")
        print(f"║                                                      ║")
        print(f"║  ⚠️  Your browser will show a security warning      ║")
        print(f"║      (self-signed certificate). Click 'Advanced'     ║")
        print(f"║      and 'Proceed to localhost'.                     ║")
        print(f"║                                                      ║")
        print(f"║  📷 Camera access now enabled!                       ║")
        print(f"║                                                      ║")
        print(f"║  Press Ctrl+C to stop                                ║")
        print(f"╚══════════════════════════════════════════════════════╝")

        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n\n👋 Server stopped.")
            sys.exit(0)


if __name__ == "__main__":
    main()

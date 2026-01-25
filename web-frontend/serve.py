#!/usr/bin/env python3
"""
Simple HTTP server for Lab2QRCode Web Application
Serves the production build with proper CORS and WASM headers
"""

import http.server
import socketserver
import sys
from pathlib import Path

PORT = 5000
DIRECTORY = Path(__file__).parent / "dist"


class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(DIRECTORY), **kwargs)

    def end_headers(self):
        # Required headers for WASM and SharedArrayBuffer
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Resource-Policy", "cross-origin")

        # WASM MIME type
        if self.path.endswith(".wasm"):
            self.send_header("Content-Type", "application/wasm")

        super().end_headers()


def main():
    if not DIRECTORY.exists():
        print(f"Error: Build directory not found: {DIRECTORY}")
        print("Run 'npm run build' first!")
        sys.exit(1)

    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        print(f"╔══════════════════════════════════════════════════════╗")
        print(f"║  Lab2QRCode Web Application                          ║")
        print(f"╠══════════════════════════════════════════════════════╣")
        print(f"║  🌐 Server running at:                               ║")
        print(f"║                                                      ║")
        print(f"║     http://localhost:{PORT}                            ║")
        print(f"║     http://127.0.0.1:{PORT}                            ║")
        print(f"║                                                      ║")
        print(f"║  📂 Serving directory: {str(DIRECTORY):<26} ║")
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

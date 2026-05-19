#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
PORT="${PORT:-8000}"
PORT_FILE="$(mktemp)"

cleanup() {
    rm -f "$PORT_FILE"
}

trap cleanup EXIT

python3 - "$PORT" "$BUILD_DIR" "$PORT_FILE" <<'PY'
import functools
import http.server
import pathlib
import socketserver
import sys

requested_port = int(sys.argv[1])
build_dir = sys.argv[2]
port_file = pathlib.Path(sys.argv[3])
handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=build_dir)

if requested_port > 0:
    port_candidates = [requested_port]
else:
    port_candidates = [0]

last_error = None
for port in port_candidates:
    try:
        with socketserver.TCPServer(("127.0.0.1", port), handler) as httpd:
            actual_port = httpd.server_address[1]
            port_file.write_text(str(actual_port), encoding="utf-8")
            print(f"Serving {build_dir} at http://127.0.0.1:{actual_port}/index.html", flush=True)
            httpd.serve_forever()
            raise SystemExit(0)
    except OSError as exc:
        last_error = exc

if requested_port > 0:
    with socketserver.TCPServer(("127.0.0.1", 0), handler) as httpd:
        actual_port = httpd.server_address[1]
        port_file.write_text(str(actual_port), encoding="utf-8")
        print(
            f"Port {requested_port} is busy, switched to http://127.0.0.1:{actual_port}/index.html",
            flush=True,
        )
        httpd.serve_forever()

raise SystemExit(last_error.errno if last_error else 1)
PY

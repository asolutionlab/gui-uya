#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
PORT="${PORT:-8000}"

cd "$BUILD_DIR"
python3 -m http.server "$PORT"

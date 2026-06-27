#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
PAGES_DIR="${PAGES_DIR:-$ROOT_DIR/build/pages}"
WEB_CJK_FONT_ASSET_NAME="${WEB_CJK_FONT_ASSET_NAME:-system_ui_cjk_font.data}"
REQUIRE_WEB_CJK_FONT_ASSET="${REQUIRE_WEB_CJK_FONT_ASSET:-1}"

require_asset() {
    local path="$1"
    if [ ! -f "$path" ]; then
        echo "error: missing web asset: $path" >&2
        exit 1
    fi
}

append_runtime_asset() {
    local path="$1"
    local existing

    [ -f "$path" ] || return 0
    for existing in "${RUNTIME_ASSETS[@]:-}"; do
        if [ "$existing" = "$path" ]; then
            return 0
        fi
    done
    RUNTIME_ASSETS+=("$path")
}

copy_runtime_asset() {
    local src="$1"
    local rel_path="${src#$BUILD_DIR/}"
    local dst="$PAGES_DIR/$rel_path"

    mkdir -p "$(dirname "$dst")"
    cp -a "$src" "$dst"
}

require_asset "$BUILD_DIR/index.html"
require_asset "$BUILD_DIR/index.js"
require_asset "$BUILD_DIR/index.wasm"
require_asset "$BUILD_DIR/index.data"
if [ "$REQUIRE_WEB_CJK_FONT_ASSET" != "0" ] && [ "$REQUIRE_WEB_CJK_FONT_ASSET" != "false" ]; then
    require_asset "$BUILD_DIR/$WEB_CJK_FONT_ASSET_NAME"
fi

rm -rf "$PAGES_DIR"
mkdir -p "$PAGES_DIR"

declare -a RUNTIME_ASSETS=()
declare -a RUNTIME_PATTERNS=(
    "index.html"
    "index.js"
    "index.wasm"
    "index.data"
    "*.data"
    "*.mem"
    "*.worker.js"
    "*.wasm.map"
    "gui/render/generated/wqy_microhei_demo_*.a8"
    "gui/render/generated/wqy_microhei_demo_*.fnt"
)

for pattern in "${RUNTIME_PATTERNS[@]}"; do
    for asset in "$BUILD_DIR"/$pattern; do
        append_runtime_asset "$asset"
    done
done

for asset in "${RUNTIME_ASSETS[@]}"; do
    copy_runtime_asset "$asset"
done

for asset in "${RUNTIME_ASSETS[@]}"; do
    require_asset "$PAGES_DIR/${asset#$BUILD_DIR/}"
done

: > "$PAGES_DIR/.nojekyll"

echo "$PAGES_DIR"

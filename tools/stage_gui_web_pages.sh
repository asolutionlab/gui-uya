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

cat > "$PAGES_DIR/doudizhu-mobile.html" <<'EOF'
<!doctype html>
<html lang="zh-CN">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
    <title>UyaGUI 斗地主手机版</title>
    <meta http-equiv="refresh" content="0; url=./?demo=doudizhu&mobile=1">
    <script>
      window.location.replace('./?demo=doudizhu&mobile=1');
    </script>
    <style>
      html, body {
        margin: 0;
        width: 100%;
        height: 100%;
        background: #0b0f16;
        color: #f4f7fb;
        font: 16px/1.5 system-ui, sans-serif;
      }

      body {
        display: flex;
        align-items: center;
        justify-content: center;
        text-align: center;
        padding: 24px;
        box-sizing: border-box;
      }

      a {
        color: #9ec1ff;
      }
    </style>
  </head>
  <body>
    <div>
      <p>正在打开横屏全屏版斗地主…</p>
      <p><a href="./?demo=doudizhu&mobile=1">如果没有自动跳转，点这里进入</a></p>
    </div>
  </body>
</html>
EOF

echo "$PAGES_DIR"

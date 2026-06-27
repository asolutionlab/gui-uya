#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
find_uya_bin() {
    if [ -n "${UYA:-}" ]; then
        printf '%s\n' "$UYA"
        return 0
    fi
    if command -v uya >/dev/null 2>&1; then
        command -v uya
        return 0
    fi
    if [ -x "$ROOT_DIR/uya/bin/uya" ]; then
        printf '%s\n' "$ROOT_DIR/uya/bin/uya"
        return 0
    fi
    printf '%s\n' "$ROOT_DIR/uya/bin/uya"
}

UYA_BIN="$(find_uya_bin)"
EMCC_BIN="${EMCC:-emcc}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
APP="${APP:-}"
OUT_NAME="${OUT_NAME:-gui_uya_web}"
OUT_C="$BUILD_DIR/${OUT_NAME}.c"
OUT_GEN_O="$BUILD_DIR/${OUT_NAME}.generated.o"
OUT_WEB_O="$BUILD_DIR/${OUT_NAME}.web_host.o"
OUT_HTML="$BUILD_DIR/index.html"
OUT_JS="$BUILD_DIR/index.js"
OUT_CIMPORT_SIDECAR="${OUT_C}imports.sh"
SHELL_FILE="${SHELL_FILE:-$ROOT_DIR/src/gui/platform/web/shell.html}"
MODE="${MODE:-debug}"
WEB_STACK_SIZE="${WEB_STACK_SIZE:-8388608}"
WEB_INITIAL_MEMORY="${WEB_INITIAL_MEMORY:-33554432}"
WEB_LEGACY_VM_SUPPORT="${WEB_LEGACY_VM_SUPPORT:-0}"
TARGET_OS="${TARGET_OS:-unknown}"
TARGET_ARCH="${TARGET_ARCH:-unknown}"
WEB_CJK_FONT_OUT="${WEB_CJK_FONT_OUT:-/app/fonts/system_ui_cjk_font}"
WEB_CJK_FONT_ASSET_NAME="${WEB_CJK_FONT_ASSET_NAME:-system_ui_cjk_font.data}"
VENDORED_WEB_CJK_FONT_SRC="$ROOT_DIR/third_party/fonts/wqy/wqy-microhei.ttc"
WEB_CJK_FONT_SRC="${WEB_CJK_FONT_SRC:-$VENDORED_WEB_CJK_FONT_SRC}"
WEB_MINIFY_HTML="${WEB_MINIFY_HTML:-auto}"
WQY_BITMAP_STAGE_DIR="$BUILD_DIR/gui/render/generated"
WQY_BITMAP_SRC_DIR="$ROOT_DIR/src/gui/render/generated"
WQY_BITMAP_FONT_SIZES=(10 11 12 13 14 15 16 17 18 20 22 24 25 26 29 31 35)

if [ -z "$APP" ]; then
    UYA_STAGE_DIR="${UYA_STAGE_DIR:-$BUILD_DIR/uya_stage}"
    python3 "$ROOT_DIR/tools/stage_uya_sources.py" --root "$ROOT_DIR" --stage "$UYA_STAGE_DIR"
    APP="$UYA_STAGE_DIR/apps/sim_web_main.uya"
fi

if ! command -v "$UYA_BIN" >/dev/null 2>&1; then
    echo "error: Uya compiler not found: $UYA_BIN" >&2
    echo "hint : install 'uya' on PATH or set UYA=/path/to/uya." >&2
    exit 127
fi

pick_emcc_stack_flag() {
    local probe_dir probe_c probe_js
    probe_dir="$(mktemp -d)"
    probe_c="$probe_dir/probe.c"
    probe_js="$probe_dir/probe.js"

    printf '%s\n' 'int main(void) { return 0; }' >"$probe_c"

    if "$EMCC_BIN" "$probe_c" -O0 -sSTACK_SIZE="$WEB_STACK_SIZE" -o "$probe_js" >/dev/null 2>&1; then
        rm -rf "$probe_dir"
        printf '%s\n' "STACK_SIZE"
        return 0
    fi

    if "$EMCC_BIN" "$probe_c" -O0 -sTOTAL_STACK="$WEB_STACK_SIZE" -o "$probe_js" >/dev/null 2>&1; then
        rm -rf "$probe_dir"
        printf '%s\n' "TOTAL_STACK"
        return 0
    fi

    rm -rf "$probe_dir"
    return 1
}

pick_web_cjk_font() {
    if [ -n "$WEB_CJK_FONT_SRC" ] && [ -f "$WEB_CJK_FONT_SRC" ]; then
        printf '%s\n' "$WEB_CJK_FONT_SRC"
        return 0
    fi

    local candidate
    # Prefer the vendored font via WEB_CJK_FONT_SRC first. If it is missing,
    # fall back to common distro fonts so local development still works.
    # Single-file TTFs probe more reliably than TTC/OTF in some runtimes.
    for candidate in \
        /usr/share/fonts/fonts-gb/GB_HT_GB18030.ttf \
        /usr/share/fonts/fonts-gb/GB_ST_GB18030.ttf \
        /usr/share/fonts/fonts-gb/GB_FS_GB18030.ttf \
        /usr/share/fonts/fonts-gb/GB_KT_GB18030.ttf \
        /usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc \
        /usr/share/fonts/opentype/source-han-cjk/SourceHanSansSC-Regular.otf \
        /usr/share/fonts/truetype/wqy/wqy-microhei.ttc \
        /usr/share/fonts/truetype/wqy/wqy-zenhei.ttc
    do
        if [ -f "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

stage_web_bitmap_fonts() {
    local size

    mkdir -p "$WQY_BITMAP_STAGE_DIR"
    rm -f "$WQY_BITMAP_STAGE_DIR"/wqy_microhei_demo_*.a8
    rm -f "$WQY_BITMAP_STAGE_DIR"/wqy_microhei_demo_*.fnt

    for size in "${WQY_BITMAP_FONT_SIZES[@]}"; do
        cp "$WQY_BITMAP_SRC_DIR/wqy_microhei_demo_${size}.fnt" "$WQY_BITMAP_STAGE_DIR/"
        cp "$WQY_BITMAP_SRC_DIR/wqy_microhei_demo_${size}.a8" "$WQY_BITMAP_STAGE_DIR/"
    done
}

transpile_web_js_legacy() {
    local babel_bin tmp_dir config_file

    if ! { command -v babeljs >/dev/null 2>&1 || command -v babel >/dev/null 2>&1; }; then
        echo "warning: WEB_LEGACY_VM_SUPPORT=1 was requested, but neither 'babeljs' nor 'babel' is available; keeping modern JS output." >&2
        return 1
    fi

    if [ ! -f "$OUT_JS" ]; then
        echo "warning: legacy JS transpile skipped because output is missing: $OUT_JS" >&2
        return 1
    fi

    babel_bin="$(command -v babeljs 2>/dev/null || command -v babel 2>/dev/null)"
    tmp_dir="$(mktemp -d)"
    config_file="$tmp_dir/babel.config.json"

    cat >"$config_file" <<'EOF'
{
  "sourceType": "script",
  "presets": [
    [
      "@babel/preset-env",
      {
        "targets": {
          "chrome": "74",
          "android": "74"
        }
      }
    ]
  ]
}
EOF

    "$babel_bin" "$OUT_JS" -o "$OUT_JS" --config-file "$config_file"
    rm -rf "$tmp_dir"
}

mkdir -p "$BUILD_DIR"

if ! command -v "$EMCC_BIN" >/dev/null 2>&1; then
    echo "error: emcc was not found. Please install Emscripten and ensure 'emcc' is on PATH." >&2
    exit 2
fi

if ! EMCC_STACK_FLAG="$(pick_emcc_stack_flag)"; then
    echo "error: could not determine a supported Emscripten stack-size flag (tried STACK_SIZE and TOTAL_STACK)." >&2
    exit 2
fi

UYA_OPT="-O0"
EMCC_OPT="-O0"
if [ "$MODE" = "release" ]; then
    UYA_OPT="-O3"
    EMCC_OPT="-O2"
fi

TARGET_OS="$TARGET_OS" TARGET_ARCH="$TARGET_ARCH" \
    "$UYA_BIN" build "$APP" --c99 --no-split-c "$UYA_OPT" -o "$OUT_C"

# Web 构建会显式把 Uya 目标切到 unknown，以屏蔽宿主 syscall/asm 路径。
# 当前 codegen 仍会在生成 C 中保留未被引用的 @syscall helper fallback #error；
# 对 wasm 目标这里直接去掉该行，避免 helper-only 诊断阻塞后续真实编译错误。
sed -i '/@syscall C99 backend: supported targets/d' "$OUT_C"
# Older distro-packaged Emscripten builds can fail to export `_main` from the
# generated Uya entry even when the symbol exists. Pin the web entry to default
# visibility so `_main` remains exportable across linker versions.
sed -i 's/^int32_t main(int32_t argc, char \*\*argv) {$/__attribute__((used, visibility("default"))) int32_t main(int32_t argc, char **argv) {/' "$OUT_C"

"$EMCC_BIN" -std=c99 "$EMCC_OPT" -fno-builtin -w \
    -include "$ROOT_DIR/src/gui/platform/web/posix_decl_shim.h" \
    -include fcntl.h \
    -include sys/uio.h \
    -include sys/mman.h \
    -c "$OUT_C" \
    -o "$OUT_GEN_O"

"$EMCC_BIN" -std=gnu99 "$EMCC_OPT" -Wall -Wextra -pedantic -fvisibility=hidden \
    -c "$ROOT_DIR/src/gui/platform/web/web_host.c" \
    -o "$OUT_WEB_O"

declare -a CIMPORT_OBJECTS=()
declare -a CIMPORT_LDFLAGS=()
declare -a HTML_MINIFY_FLAGS=()
declare -a WEB_COMPAT_FLAGS=()
declare -a PRELOAD_FILES=(
    --preload-file "$ROOT_DIR/.uya_sim_root_probe@/app/.uya_sim_root_probe"
)

if [ "$WEB_LEGACY_VM_SUPPORT" = "1" ] || [ "$WEB_LEGACY_VM_SUPPORT" = "true" ]; then
    # Debian's emscripten package cannot run its own transpile step reliably,
    # so we lower the Chrome feature target without enabling POLYFILL-driven
    # auto-transpile, then post-process index.js with babeljs below.
    WEB_COMPAT_FLAGS+=(-sPOLYFILL=0 -sMIN_CHROME_VERSION=74)
fi

if [ "$WEB_MINIFY_HTML" = "0" ] || [ "$WEB_MINIFY_HTML" = "false" ]; then
    HTML_MINIFY_FLAGS+=(-sMINIFY_HTML=0)
elif [ "$WEB_MINIFY_HTML" = "auto" ]; then
    # Some distro-packaged Emscripten releases enable HTML minification for
    # optimized HTML builds, but do not depend on html-minifier-terser.
    if ! command -v html-minifier-terser >/dev/null 2>&1; then
        HTML_MINIFY_FLAGS+=(-sMINIFY_HTML=0)
        echo "warning: html-minifier-terser was not found; disabling HTML minification for web build." >&2
    fi
fi

if WEB_CJK_FONT_PICKED="$(pick_web_cjk_font)"; then
    cp "$WEB_CJK_FONT_PICKED" "$BUILD_DIR/$WEB_CJK_FONT_ASSET_NAME"
    echo "info: staged web CJK font asset: $WEB_CJK_FONT_PICKED -> $BUILD_DIR/$WEB_CJK_FONT_ASSET_NAME (runtime mount: $WEB_CJK_FONT_OUT)" >&2
else
    rm -f "$BUILD_DIR/$WEB_CJK_FONT_ASSET_NAME"
    echo "warning: no scalable CJK font found for web build; browser fallback will stay on builtin 8x8 glyphs." >&2
fi

stage_web_bitmap_fonts
echo "info: staged external web bitmap font assets under: $WQY_BITMAP_STAGE_DIR" >&2

if [ -f "$OUT_CIMPORT_SIDECAR" ]; then
    # shellcheck disable=SC1090
    . "$OUT_CIMPORT_SIDECAR"

    ci=0
    while [ "$ci" -lt "${UYA_CIMPORT_COUNT:-0}" ]; do
        src_var="UYA_CIMPORT_SRC_${ci}"
        cflagc_var="UYA_CIMPORT_CFLAGC_${ci}"
        src_path="${!src_var}"
        cflagc="${!cflagc_var:-0}"
        obj_path="$BUILD_DIR/${OUT_NAME}.cimport.${ci}.o"

        compile_cmd=("$EMCC_BIN" -std=c99 "$EMCC_OPT" -fno-builtin -fvisibility=hidden)
        cj=0
        while [ "$cj" -lt "$cflagc" ]; do
            cflag_var="UYA_CIMPORT_CFLAG_${ci}_${cj}"
            compile_cmd+=("${!cflag_var}")
            cj=$((cj + 1))
        done
        compile_cmd+=(-c "$src_path" -o "$obj_path")
        "${compile_cmd[@]}"
        CIMPORT_OBJECTS+=("$obj_path")
        ci=$((ci + 1))
    done

    ldflagc="${UYA_CIMPORT_LDFLAGC:-0}"
    li=0
    while [ "$li" -lt "$ldflagc" ]; do
        ldflag_var="UYA_CIMPORT_LDFLAG_${li}"
        CIMPORT_LDFLAGS+=("${!ldflag_var}")
        li=$((li + 1))
    done
fi

# Use an argument array so optional flags like MINIFY_HTML survive shell
# parsing exactly as intended across distro-specific emcc wrappers.
declare -a LINK_CMD=(
    "$EMCC_BIN"
    "$EMCC_OPT"
    "$OUT_GEN_O"
    "$OUT_WEB_O"
    "${CIMPORT_OBJECTS[@]}"
    -o "$OUT_HTML"
    -sALLOW_MEMORY_GROWTH=1
    -sINITIAL_MEMORY="$WEB_INITIAL_MEMORY"
    -sNO_EXIT_RUNTIME=1
    -s"$EMCC_STACK_FLAG"="$WEB_STACK_SIZE"
    -sEXPORTED_FUNCTIONS=_main,_malloc,_free,_uya_gui_web_host_feed_event,_uya_gui_web_host_feed_text_input
    -lidbfs.js
    --shell-file "$SHELL_FILE"
    "${WEB_COMPAT_FLAGS[@]}"
    "${HTML_MINIFY_FLAGS[@]}"
    "${PRELOAD_FILES[@]}"
    "${CIMPORT_LDFLAGS[@]}"
)

"${LINK_CMD[@]}"

if [ "$WEB_LEGACY_VM_SUPPORT" = "1" ] || [ "$WEB_LEGACY_VM_SUPPORT" = "true" ]; then
    transpile_web_js_legacy || true
fi

echo "$OUT_HTML"

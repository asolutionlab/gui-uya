#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
UYA_BIN="${UYA:-$ROOT_DIR/uya/bin/uya}"
EMCC_BIN="${EMCC:-emcc}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/web}"
APP="${APP:-$ROOT_DIR/gui/sim_web_main.uya}"
OUT_NAME="${OUT_NAME:-gui_uya_web}"
OUT_C="$BUILD_DIR/${OUT_NAME}.c"
OUT_GEN_O="$BUILD_DIR/${OUT_NAME}.generated.o"
OUT_WEB_O="$BUILD_DIR/${OUT_NAME}.web_host.o"
OUT_HTML="$BUILD_DIR/index.html"
OUT_CIMPORT_SIDECAR="${OUT_C}imports.sh"
SHELL_FILE="${SHELL_FILE:-$ROOT_DIR/gui/platform/web/shell.html}"
MODE="${MODE:-debug}"
WEB_STACK_SIZE="${WEB_STACK_SIZE:-8388608}"
TARGET_OS="${TARGET_OS:-unknown}"
TARGET_ARCH="${TARGET_ARCH:-unknown}"
WEB_CJK_FONT_OUT="${WEB_CJK_FONT_OUT:-/app/fonts/system_ui_cjk_font}"
VENDORED_WEB_CJK_FONT_SRC="$ROOT_DIR/third_party/fonts/wqy/wqy-microhei.ttc"
WEB_CJK_FONT_SRC="${WEB_CJK_FONT_SRC:-$VENDORED_WEB_CJK_FONT_SRC}"
WEB_MINIFY_HTML="${WEB_MINIFY_HTML:-auto}"

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

mkdir -p "$BUILD_DIR"

if ! command -v "$EMCC_BIN" >/dev/null 2>&1; then
    echo "error: emcc was not found. Please install Emscripten and ensure 'emcc' is on PATH." >&2
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
sed -i '/#include <math.h>/a extern ssize_t write(int fd, const char *buf, size_t count);\nextern ssize_t read(int fd, char *buf, size_t count);\nextern int close(int fd);\nextern int access(const char *pathname, int mode);\nextern int64_t lseek(int fd, int64_t offset, int whence);\nextern void _exit(int code);' "$OUT_C"

"$EMCC_BIN" -std=c99 "$EMCC_OPT" -fno-builtin -fvisibility=hidden -w \
    -include fcntl.h \
    -include sys/mman.h \
    -c "$OUT_C" \
    -o "$OUT_GEN_O"

"$EMCC_BIN" -std=gnu99 "$EMCC_OPT" -Wall -Wextra -pedantic -fvisibility=hidden \
    -c "$ROOT_DIR/gui/platform/web/web_host.c" \
    -o "$OUT_WEB_O"

declare -a CIMPORT_OBJECTS=()
declare -a CIMPORT_LDFLAGS=()
declare -a HTML_MINIFY_FLAGS=()
declare -a PRELOAD_FILES=(
    --preload-file "$ROOT_DIR/gui@/app/gui"
    --preload-file "$ROOT_DIR/.uya_sim_root_probe@/app/.uya_sim_root_probe"
)

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
    PRELOAD_FILES+=(--preload-file "$WEB_CJK_FONT_PICKED@$WEB_CJK_FONT_OUT")
    echo "info: preload web CJK font: $WEB_CJK_FONT_PICKED -> $WEB_CJK_FONT_OUT" >&2
else
    echo "warning: no scalable CJK font found for web build; browser fallback will stay on builtin 8x8 glyphs." >&2
fi

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

"$EMCC_BIN" "$EMCC_OPT" \
    "$OUT_GEN_O" \
    "$OUT_WEB_O" \
    "${CIMPORT_OBJECTS[@]}" \
    -o "$OUT_HTML" \
    -sALLOW_MEMORY_GROWTH=1 \
    -sNO_EXIT_RUNTIME=1 \
    -Wl,-z,stack-size="$WEB_STACK_SIZE" \
    -sEXPORTED_FUNCTIONS=_main,_uya_gui_web_host_feed_event \
    -lidbfs.js \
    --shell-file "$SHELL_FILE" \
    "${HTML_MINIFY_FLAGS[@]}" \
    "${PRELOAD_FILES[@]}" \
    "${CIMPORT_LDFLAGS[@]}"

echo "$OUT_HTML"

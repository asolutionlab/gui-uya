#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
UYA_BIN="${UYA:-$ROOT_DIR/uya/bin/uya}"
CC_BIN="${CC:-cc}"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build/sim}"
APP="${APP:-$ROOT_DIR/gui/sim_main.uya}"
OUT_NAME="${OUT_NAME:-gui_uya_sim}"
OUT_C="$BUILD_DIR/${OUT_NAME}.c"
OUT_BIN="$BUILD_DIR/$OUT_NAME"
OUT_GEN_O="$BUILD_DIR/${OUT_NAME}.generated.o"
OUT_SDL_O="$BUILD_DIR/${OUT_NAME}.sdl_host.o"
OUT_FB_O="$BUILD_DIR/${OUT_NAME}.fb_host.o"
OUT_CIMPORT_SIDECAR="${OUT_C}imports.sh"
MODE="${MODE:-debug}"

mkdir -p "$BUILD_DIR"

SDL_CFLAGS_STR=""
SDL_LIBS_STR=""
if pkg-config --exists sdl2 2>/dev/null; then
    SDL_CFLAGS_STR="$(pkg-config --cflags sdl2)"
    SDL_LIBS_STR="$(pkg-config --libs sdl2)"
elif command -v sdl2-config >/dev/null 2>&1; then
    SDL_CFLAGS_STR="$(sdl2-config --cflags)"
    SDL_LIBS_STR="$(sdl2-config --libs)"
else
    echo "error: SDL2 development files were not found." >&2
    echo "hint : install libsdl2-dev (Debian/Ubuntu) or SDL2-devel (Fedora/RHEL), then rerun 'make sim-build'." >&2
    exit 2
fi

UYA_OPT="-O0"
CC_OPT="-O0"
if [ "$MODE" = "release" ]; then
    UYA_OPT="-O3"
    CC_OPT="-O2"
fi

read -r -a SDL_CFLAGS <<<"$SDL_CFLAGS_STR"
read -r -a SDL_LIBS <<<"$SDL_LIBS_STR"

OPENAI_HOST_C="$ROOT_DIR/gui/platform/openai/openai_chat_host.c"
OPENAI_OBJ=""
OPENAI_CFLAGS_STR=""
OPENAI_LIBS_STR=""
if pkg-config --exists libcurl 2>/dev/null; then
    OUT_OPENAI_O="$BUILD_DIR/${OUT_NAME}.openai_chat.o"
    OPENAI_OBJ="$OUT_OPENAI_O"
    OPENAI_CFLAGS_STR="$(pkg-config --cflags libcurl)"
    OPENAI_LIBS_STR="$(pkg-config --libs libcurl)"
fi

read -r -a OPENAI_CFLAGS <<<"$OPENAI_CFLAGS_STR"
read -r -a OPENAI_LIBS <<<"$OPENAI_LIBS_STR"

"$UYA_BIN" build "$APP" --c99 --no-split-c "$UYA_OPT" -o "$OUT_C"

HOST_C="$ROOT_DIR/gui/platform/sdl2/sdl_host.c"
FB_HOST_C="$ROOT_DIR/gui/platform/fb/fb_host.c"

# 生成的 Uya C 代码会携带大量与宿主 libc / compiler 内建相关的噪声 warning，
# 这里静默编译生成物，只保留我们手写 host glue 的真实 warning。
"$CC_BIN" -std=c99 -g "$CC_OPT" -fno-builtin -fvisibility=hidden -w \
    "${SDL_CFLAGS[@]}" \
    -c "$OUT_C" \
    -o "$OUT_GEN_O"

"$CC_BIN" -std=c99 -Wall -Wextra -pedantic -g "$CC_OPT" -fvisibility=hidden \
    "${SDL_CFLAGS[@]}" \
    -c "$HOST_C" \
    -o "$OUT_SDL_O"

"$CC_BIN" -std=c99 -Wall -Wextra -pedantic -g "$CC_OPT" -fvisibility=hidden \
    "${SDL_CFLAGS[@]}" \
    -c "$FB_HOST_C" \
    -o "$OUT_FB_O"

if [ -n "$OPENAI_OBJ" ]; then
    "$CC_BIN" -std=c99 -Wall -Wextra -pedantic -g "$CC_OPT" -fvisibility=hidden \
        "${OPENAI_CFLAGS[@]}" \
        -c "$OPENAI_HOST_C" \
        -o "$OUT_OPENAI_O"
fi

declare -a CIMPORT_OBJECTS=()
declare -a CIMPORT_LDFLAGS=()

if [ -f "$OUT_CIMPORT_SIDECAR" ]; then
    # shellcheck disable=SC1090
    . "$OUT_CIMPORT_SIDECAR"

    ci=0
    while [ "$ci" -lt "${UYA_CIMPORT_COUNT:-0}" ]; do
        src_var="UYA_CIMPORT_SRC_${ci}"
        cflagc_var="UYA_CIMPORT_CFLAGC_${ci}"
        src_path="${!src_var}"
        cflagc="${!cflagc_var:-0}"
        obj_path="$BUILD_DIR/gui_uya_sim.cimport.${ci}.o"

        compile_cmd=("$CC_BIN" -std=c99 -g "$CC_OPT" -fno-builtin -fvisibility=hidden)
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

"$CC_BIN" -g "$CC_OPT" \
    "$OUT_GEN_O" \
    "$OUT_SDL_O" \
    "$OUT_FB_O" \
    $OPENAI_OBJ \
    "${CIMPORT_OBJECTS[@]}" \
    -o "$OUT_BIN" \
    "${SDL_LIBS[@]}" \
    "${OPENAI_LIBS[@]}" \
    -pthread \
    -ldl \
    -lm \
    "${CIMPORT_LDFLAGS[@]}"

echo "$OUT_BIN"

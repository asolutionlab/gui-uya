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
MODE="${MODE:-debug}"

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

"$UYA_BIN" build "$APP" --c99 --no-split-c "$UYA_OPT" -o "$OUT_C"

"$EMCC_BIN" -std=c99 "$EMCC_OPT" -fno-builtin -fvisibility=hidden -w \
    -c "$OUT_C" \
    -o "$OUT_GEN_O"

"$EMCC_BIN" -std=c99 "$EMCC_OPT" -Wall -Wextra -pedantic -fvisibility=hidden \
    -c "$ROOT_DIR/gui/platform/web/web_host.c" \
    -o "$OUT_WEB_O"

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
    -sEXPORTED_FUNCTIONS=_main,_uya_gui_web_host_feed_event \
    --preload-file "$ROOT_DIR/gui@/app/gui" \
    --preload-file "$ROOT_DIR/.uya_sim_root_probe@/app/.uya_sim_root_probe" \
    "${CIMPORT_LDFLAGS[@]}"

echo "$OUT_HTML"

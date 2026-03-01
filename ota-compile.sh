#!/usr/bin/env bash
#
# ota-compile.sh — OTA Security Compiler Driver
#
# This script acts as a drop-in compiler wrapper that integrates
# the OTA security analysis pass into the standard compilation
# pipeline.  It replaces the typical  clang → object  flow with:
#
#   ┌──────────┐    ┌──────────┐    ┌──────────────┐    ┌──────────┐
#   │  C Source │───>│ Clang    │───>│ opt + OTA    │───>│ Object / │
#   │  (.c)    │    │ emit-llvm│    │ SecurityPass │    │ Binary   │
#   └──────────┘    └──────────┘    └──────────────┘    └──────────┘
#      input          → .ll             analysis           output
#                                     + enforcement
#
# USAGE:
#   ./ota-compile.sh <source.c> [--emit-ll] [--no-compile] [--verbose]
#
# OPTIONS:
#   --emit-ll      Also keep the .ll (LLVM IR) output
#   --no-compile   Stop after analysis, don't produce a binary
#   --verbose      Show all intermediate commands
#   --help         Print this help
#
# REQUIREMENTS:
#   - clang (14+)
#   - opt   (matching LLVM version)
#   - OTASecurityPass.so built in llvm-pass/build/
#

set -euo pipefail

# ------------------------------------------------------------------ #
#  Configuration                                                      #
# ------------------------------------------------------------------ #

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PASS_DIR="${SCRIPT_DIR}/llvm-pass/build"
PASS_LIB="${PASS_DIR}/OTASecurityPass.so"
LOG_FILE="${SCRIPT_DIR}/secure_log.txt"

CLANG="${CLANG:-clang}"
OPT="${OPT:-opt}"

EMIT_LL=false
NO_COMPILE=false
VERBOSE=false

# ------------------------------------------------------------------ #
#  Argument Parsing                                                    #
# ------------------------------------------------------------------ #

usage() {
    head -35 "$0" | tail -25
    exit 0
}

INPUT_FILE=""
for arg in "$@"; do
    case "$arg" in
        --emit-ll)    EMIT_LL=true ;;
        --no-compile) NO_COMPILE=true ;;
        --verbose)    VERBOSE=true ;;
        --help|-h)    usage ;;
        *)            INPUT_FILE="$arg" ;;
    esac
done

if [ -z "$INPUT_FILE" ]; then
    echo "Error: No input file specified."
    echo "Usage: $0 <source.c> [--emit-ll] [--no-compile] [--verbose]"
    exit 1
fi

if [ ! -f "$INPUT_FILE" ]; then
    echo "Error: File not found: $INPUT_FILE"
    exit 1
fi

if [ ! -f "$PASS_LIB" ]; then
    echo "Error: OTASecurityPass.so not found at $PASS_LIB"
    echo "Build it first:  cd llvm-pass/build && cmake .. && make"
    exit 1
fi

BASENAME="$(basename "$INPUT_FILE" .c)"
DIR="$(dirname "$INPUT_FILE")"
LL_FILE="${DIR}/${BASENAME}.ll"
OPT_FILE="${DIR}/${BASENAME}.opt.ll"
OBJ_FILE="${DIR}/${BASENAME}.o"
BIN_FILE="${DIR}/${BASENAME}"

run_cmd() {
    if $VERBOSE; then
        echo "+ $*"
    fi
    "$@"
}

# ------------------------------------------------------------------ #
#  Pipeline Stage 1: C → LLVM IR                                      #
# ------------------------------------------------------------------ #

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║          OTA Security Compiler — Pipeline               ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "[STAGE 1/4] Compiling C to LLVM IR..."
run_cmd $CLANG -S -emit-llvm -O0 -g "$INPUT_FILE" -o "$LL_FILE"
echo "           → $LL_FILE"

# ------------------------------------------------------------------ #
#  Pipeline Stage 2: AST-Level Analysis (optional, if plugin built)   #
# ------------------------------------------------------------------ #

AST_PLUGIN="${SCRIPT_DIR}/ast/build/UpdateASTPass.so"
if [ -f "$AST_PLUGIN" ]; then
    echo ""
    echo "[STAGE 2/4] Running AST-level security analysis..."
    run_cmd $CLANG -fplugin="$AST_PLUGIN" -fsyntax-only "$INPUT_FILE" 2>&1 || true
    echo "           AST analysis complete."
else
    echo ""
    echo "[STAGE 2/4] AST plugin not built — skipping."
    echo "           (Build it in ast/build/ to enable source-level checks)"
fi

# ------------------------------------------------------------------ #
#  Pipeline Stage 3: LLVM IR Security Analysis                        #
# ------------------------------------------------------------------ #

echo ""
echo "[STAGE 3/4] Running OTA Security Pass on LLVM IR..."
run_cmd $OPT -load-pass-plugin "$PASS_LIB" \
    -passes="ota-security" \
    -disable-output \
    "$LL_FILE" 2>&1

echo ""
echo "           Security log → $LOG_FILE"

# ------------------------------------------------------------------ #
#  Pipeline Stage 4: Compile to binary (unless --no-compile)          #
# ------------------------------------------------------------------ #

if $NO_COMPILE; then
    echo ""
    echo "[STAGE 4/4] Skipped (--no-compile)."
else
    echo ""
    echo "[STAGE 4/4] Compiling to native binary..."
    run_cmd $CLANG "$LL_FILE" -o "$BIN_FILE"
    echo "           → $BIN_FILE"
fi

# ------------------------------------------------------------------ #
#  Cleanup                                                             #
# ------------------------------------------------------------------ #

if ! $EMIT_LL; then
    rm -f "$LL_FILE" "$OPT_FILE"
fi

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                    Pipeline Complete                    ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Return non-zero if the log contains FAIL entries
if grep -q "\[FAIL\]" "$LOG_FILE" 2>/dev/null; then
    echo "⚠  Security violations detected. See $LOG_FILE"
    exit 1
else
    echo "✓  All security checks passed."
    exit 0
fi

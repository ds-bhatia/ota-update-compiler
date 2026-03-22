#!/usr/bin/env bash
set -euo pipefail

CLANG_EXE="clang"
OPT_EXE="opt"
TESTS_DIR="tests"
PLUGIN_PATH=""

usage() {
  echo "Usage: scripts/run_policy_matrix.sh [--clang clang] [--opt opt] [--tests-dir tests] [--plugin /path/to/libTraversalPass.so]"
}

resolve_plugin_path() {
  local explicit="$1"
  if [[ -n "$explicit" && -f "$explicit" ]]; then
    echo "$explicit"
    return 0
  fi

  local candidates=(
    "llvm-pass/build/libTraversalPass.so"
    "llvm-pass/build/TraversalPass.so"
    "llvm-pass/build/Release/libTraversalPass.so"
    "llvm-pass/build/Debug/libTraversalPass.so"
  )

  local c
  for c in "${candidates[@]}"; do
    [[ -f "$c" ]] && { echo "$c"; return 0; }
  done

  return 1
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --clang)
      CLANG_EXE="$2"
      shift 2
      ;;
    --opt)
      OPT_EXE="$2"
      shift 2
      ;;
    --tests-dir)
      TESTS_DIR="$2"
      shift 2
      ;;
    --plugin)
      PLUGIN_PATH="$2"
      shift 2
      ;;
    --help|-h)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 2
      ;;
  esac
done

if ! PLUGIN_RESOLVED="$(resolve_plugin_path "$PLUGIN_PATH")"; then
  echo "Unable to find pass plugin. Build it first or pass --plugin." >&2
  exit 2
fi

if [[ ! -d "$TESTS_DIR" ]]; then
  echo "Tests directory not found: $TESTS_DIR" >&2
  exit 2
fi

mapfile -t TEST_FILES < <(find "$TESTS_DIR" -maxdepth 1 -type f -name "*.c" | sort)
if [[ ${#TEST_FILES[@]} -eq 0 ]]; then
  echo "No .c files found in $TESTS_DIR" >&2
  exit 2
fi

echo "Using plugin: $PLUGIN_RESOLVED"
echo

failures=0
total=0

for cfile in "${TEST_FILES[@]}"; do
  base="$(basename "$cfile" .c)"
  llfile="$TESTS_DIR/$base.ll"
  expected="skip"

  if [[ "$base" == secure* ]]; then
    expected="pass"
  elif [[ "$base" == insecure* ]]; then
    expected="fail"
  fi

  if [[ "$expected" == "skip" ]]; then
    echo "[SKIP] $(basename "$cfile") (no secure/insecure prefix)"
    continue
  fi

  total=$((total + 1))

  if ! "$CLANG_EXE" -S -emit-llvm "$cfile" -o "$llfile" >/dev/null 2>&1; then
    echo "[FAIL] $(basename "$cfile"): clang failed"
    failures=$((failures + 1))
    continue
  fi

  if "$OPT_EXE" -load-pass-plugin "$PLUGIN_RESOLVED" -passes=traversal-pass -disable-output "$llfile" >/dev/null 2>&1; then
    actual="pass"
  else
    actual="fail"
  fi

  if [[ "$actual" == "$expected" ]]; then
    echo "[OK]   $(basename "$cfile"): expected $expected, got $actual"
  else
    echo "[FAIL] $(basename "$cfile"): expected $expected, got $actual"
    failures=$((failures + 1))
  fi
done

echo
echo "Checked: $total files"
if [[ "$failures" -gt 0 ]]; then
  echo "Matrix result: FAILED ($failures mismatches)"
  exit 1
fi

echo "Matrix result: PASSED"
exit 0

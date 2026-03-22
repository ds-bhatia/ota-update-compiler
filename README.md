# Secure OTA Update Compiler

Compiler extension for LLVM/Clang that enforces OTA firmware security invariants at compile time for C update logic, focused on updateFirmware().

## What It Enforces

Inside updateFirmware(), the pass fails compilation when it detects:

- install path not dominated by signature verification.
- install path not dominated by trusted source validation.
- rollback guard not gating install path (expected logic: new_version > current_version).
- sensitive logging APIs in updateFirmware().
- weak APIs in updateFirmware() (for example MD5, SHA1, rand).

## Repository Layout

- llvm-pass/: LLVM new-pass-manager plugin that performs enforcement.
- ast/: Clang AST plugin prototype.
- tests/: secure and insecure OTA firmware examples, including Week 3 rule matrix.
- scripts/: reproducible command wrappers for matrix execution.

## Prerequisites

Install and make available on PATH:

- CMake 3.13+
- LLVM toolchain matching your headers/libraries
- clang
- opt

Tip: use the same LLVM/Clang major version for building and running the pass.

## Build LLVM Pass Plugin

From repository root:

```powershell
cmake -S llvm-pass -B llvm-pass/build
cmake --build llvm-pass/build --config Release
```

## Run Single File Check

Example with secure sample:

```powershell
clang -S -emit-llvm tests/secure.c -o tests/secure.ll
opt -load-pass-plugin llvm-pass/build/TraversalPass.dll -passes=traversal-pass -disable-output tests/secure.ll
```

Notes:

- On Linux/macOS, plugin extension is usually .so.
- If your build generates plugin in another location, pass the full plugin path.

## Run Week 3 Matrix (One Command)

From repository root:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_policy_matrix.ps1
```

Behavior:

- Compiles each tests/*.c file to LLVM IR.
- Runs the OTA security pass.
- Treats files beginning with secure as expected pass.
- Treats files beginning with insecure as expected fail.
- Returns non-zero if any expectation is violated.

Optional parameters:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/run_policy_matrix.ps1 -ClangExe clang -OptExe opt
```

## Week Status

- Week 1: completed (core policy enforcement)
- Week 2: completed (realistic secure/insecure firmware samples)
- Week 3: completed (focused violation matrix)
- Week 4: completed (path-gated rollback precision + better diagnostics)
- Week 5: completed (build and demo packaging)
- Week 6: pending (final validation report)
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

- Ubuntu 22.04+ (or compatible Linux distro)
- CMake 3.13+
- LLVM toolchain matching your headers/libraries
- clang
- opt

Tip: use the same LLVM/Clang major version for building and running the pass.

## Build LLVM Pass Plugin

From repository root:

```bash
cmake -S llvm-pass -B llvm-pass/build
cmake --build llvm-pass/build
```

## Run Single File Check

Example with secure sample:

```bash
clang -S -emit-llvm tests/secure.c -o tests/secure.ll
opt -load-pass-plugin llvm-pass/build/libTraversalPass.so -passes=traversal-pass -disable-output tests/secure.ll
```

Notes:

- Plugin output may be libTraversalPass.so or TraversalPass.so depending on toolchain.

## secure-clang Driver

The project includes a clang wrapper so usage feels like a compiler command:

```bash
./secure-clang tests/secure.c -o secure.out
```

Behavior:

- Generates temporary LLVM IR for each input .c file.
- Runs the OTA security pass through opt.
- If policy checks pass, forwards original arguments to clang.
- If policy checks fail, compilation is blocked.
- Prints CodeCarbon energy and emissions metrics per phase and total run.

To use as a global command on Ubuntu:

```bash
chmod +x secure-clang scripts/run_policy_matrix.sh
sudo ln -sf "$(pwd)/secure-clang" /usr/local/bin/secure-clang
```

Optional explicit tool paths:

```bash
secure-clang --clang clang --opt opt --plugin llvm-pass/build/libTraversalPass.so -- tests/secure.c -o secure.out
```

## Run Week 3 Matrix (One Command)

From repository root:

```bash
./scripts/run_policy_matrix.sh
```

Behavior:

- Compiles each tests/*.c file to LLVM IR.
- Runs the OTA security pass.
- Treats files beginning with secure as expected pass.
- Treats files beginning with insecure as expected fail.
- Returns non-zero if any expectation is violated.

Optional parameters:

```bash
./scripts/run_policy_matrix.sh --clang clang --opt opt --plugin llvm-pass/build/libTraversalPass.so
```

## Web Demo Interface

A browser-based demo UI is available in web-demo/ to load firmware code, run secure-clang, and visualize violations.

Run it:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r web-demo/requirements.txt
python3 web-demo/app.py
```

Then open:

```text
http://localhost:5050
```

Features:

- Load any sample from tests/*.c.
- Edit or paste your own firmware update code.
- Run secure-clang from the UI.
- View policy violations and line-level hints.
- View phase-wise and total CodeCarbon energy metrics for each compile run.
- Use Demo Mode one-click scenarios for live presentation.
- Track run history with recent compile outcomes and energy trend chart.
- Export run reports as JSON and Markdown directly from the UI.

### CodeCarbon Notes

secure-clang automatically uses CodeCarbon when available in the active Python environment.

CLI output lines include:

- [CodeCarbon] phase=... energy_kwh=... emissions_kg=...
- [CodeCarbon] total energy_kwh=... emissions_kg=...

If CodeCarbon is not installed, secure-clang still runs, but energy lines are not emitted.


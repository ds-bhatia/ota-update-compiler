# OTA Security Compiler

A static analysis compiler that detects insecure OTA (Over-The-Air) firmware update code using LLVM and Clang infrastructure.

**Capstone project for Compiler Design (CS1202).**

---

## Overview

This compiler integrates into the standard C compilation pipeline and enforces four security invariants on firmware update routines:

| Rule | Invariant | What It Catches |
|------|-----------|-----------------|
| RULE-1 | `verifySignature()` must dominate `install()` | Missing cryptographic signature verification |
| RULE-2 | Version comparison must dominate `install()` | No rollback prevention (firmware downgrade attacks) |
| RULE-3 | `sourceTrusted()` must dominate `install()` | No server/source trust validation |
| RULE-4 | `install()` only via conditional branches | Unconditionally reachable install path |

## Architecture

```
┌──────────┐    ┌──────────────┐    ┌───────────────────┐    ┌──────────┐
│  C Source │───>│ Clang        │───>│ opt + OTA         │───>│ Binary / │
│  (.c)    │    │ (emit-llvm)  │    │ SecurityPass      │    │ Report   │
└──────────┘    └──────────────┘    └───────────────────┘    └──────────┘
       │               │                    │
       │        LLVM IR (.ll)        CFG analysis
       │                            Dominator tree
       │                            Rule enforcement
       │                            secure_log.txt
       │
       └─── AST Plugin (optional source-level checks)
```

### Components

| Component | Path | Purpose |
|-----------|------|---------|
| **OTA Security Pass** | `llvm-pass/TraversalPass.cpp` | LLVM IR-level CFG traversal, dominator analysis, rule enforcement |
| **AST Plugin** | `ast/UpdateASTPass.cpp` | Clang AST-level security call detection and ordering checks |
| **Compiler Driver** | `ota-compile.sh` | Pipeline orchestrator (C → IR → Analysis → Binary) |
| **Test: Secure** | `tests/secure.c` | Properly secured firmware update routine |
| **Test: Insecure** | `tests/insecure.c` | Vulnerable firmware update with all violations |

## Quick Start

### Prerequisites

- LLVM/Clang 14+ (with `opt`, `clang`, `llvm-config`)
- CMake 3.13+
- Make

### Build

```bash
# Build the LLVM pass
cd llvm-pass/build
cmake ..
make

# Or from the project root:
make build
```

### Run Analysis

```bash
# Using the compiler driver (recommended)
./ota-compile.sh tests/insecure.c --emit-ll --no-compile

# Using Make targets
make analyze-secure       # Should report ALL PASS
make analyze-insecure     # Should report 4 FAIL

# Run both
make test

# Manual pipeline
clang -S -emit-llvm -O0 tests/insecure.c -o tests/insecure.ll
opt -load-pass-plugin llvm-pass/build/OTASecurityPass.so \
    -passes="ota-security" \
    -disable-output \
    tests/insecure.ll
cat secure_log.txt
```

### Expected Output

**For `secure.c`** (all checks present):
```
[PASS] RULE-1: Signature verification (verifySignature dominates install)
[PASS] RULE-2: Rollback prevention (version comparison dominates install)
[PASS] RULE-3: Source trust validation (sourceTrusted dominates install)
[PASS] RULE-4: Conditional guard (install reachable only via conditional branch)

RESULT: ALL CHECKS PASSED (4/4)
Firmware update code is SECURE.
```

**For `insecure.c`** (no checks present):
```
[FAIL] RULE-1: Signature verification (verifySignature dominates install)
[FAIL] RULE-2: Rollback prevention (version comparison dominates install)
[FAIL] RULE-3: Source trust validation (sourceTrusted dominates install)
[FAIL] RULE-4: Conditional guard (install reachable only via conditional branch)

RESULT: 4 VIOLATION(S) DETECTED (0/4 passed)
Firmware update code is INSECURE.
```

## Project Structure

```
ota-update-compiler/
├── ota-compile.sh              # Compiler driver script
├── Makefile                    # Build & test automation
├── secure_log.txt              # Generated audit report
├── README.md
│
├── llvm-pass/
│   ├── TraversalPass.cpp       # LLVM IR security enforcement pass
│   ├── CMakeLists.txt
│   └── build/                  # Build output (OTASecurityPass.so)
│
├── ast/
│   └── UpdateASTPass.cpp       # Clang AST analysis plugin
│
├── tests/
│   ├── secure.c                # Secure firmware update (should PASS)
│   ├── insecure.c              # Insecure firmware update (should FAIL)
│   ├── secure.ll               # Generated LLVM IR
│   └── insecure.ll             # Generated LLVM IR
│
└── read/                       # Weekly project documentation
    ├── CD Project Plan.pdf
    ├── Week 1/ ... Week 8/
```

## Technology Stack

- **LLVM** — IR representation, `opt` pass infrastructure, dominator tree analysis
- **Clang** — C frontend, AST visitor framework, plugin system
- **CMake** — Build system for the shared library pass
- **C** — Target language for firmware analysis
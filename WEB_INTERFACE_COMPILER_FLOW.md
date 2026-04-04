# Web Interface to Compiler Flow

## Purpose

This document explains how the web interface executes the secure OTA compiler pipeline and where each check happens.

## Components

- web-demo/templates/index.html: UI layout (editor, controls, output panels).
- web-demo/static/app.js: Browser logic for loading samples and sending compile requests.
- web-demo/app.py: Flask backend API and secure-clang orchestration.
- secure-clang: Compiler wrapper that runs LLVM pass checks before final clang compilation.
- llvm-pass/TraversalPass.cpp: LLVM pass that enforces OTA security invariants.

## End-to-End Request Flow

1. User opens the web interface at http://localhost:5050.
2. Browser loads a sample from tests/*.c using GET /api/samples and GET /api/samples/<name>.
3. User clicks Run secure-clang.
4. Browser sends POST /api/compile with:
   - filename
   - code (full C source)
5. Flask backend writes the source into a temporary .c file.
6. Backend calls secure-clang with the temp file.
7. secure-clang performs 3 phases:
   - clang-ir: generate temporary LLVM IR from C source.
   - opt-policy: run TraversalPass (security enforcement) over IR.
   - clang-final: produce final output binary only if policy checks pass.
8. secure-clang emits:
   - compiler diagnostics
   - policy violations from LLVM pass
   - CodeCarbon energy lines per phase and total
9. Flask parses output into structured JSON:
   - status (passed/failed/setup-error)
   - violations list
   - syntax/policy hints
   - raw output
   - energy metrics (phase + total)
10. Browser renders all diagnostics and energy details in the output panel.

## Where Security Enforcement Happens

Security policy enforcement is done in llvm-pass/TraversalPass.cpp during the opt-policy phase.

The pass checks updateFirmware() for:

- signature verification dominance before install,
- trusted source validation dominance before install,
- rollback guard that gates install path,
- sensitive logging API usage,
- weak crypto/entropy API usage.

If any rule is violated, secure-clang blocks compilation.

## Why This Is Still a Compiler Demo

The web app is only a front-end shell.

- It does not implement security checks itself.
- It forwards code into the compiler wrapper.
- All security decisions come from the LLVM pass.

This keeps the demo aligned with compiler-level enforcement rather than runtime-only validation.

## Error and Energy Visualization

The UI shows:

- human-readable pass/fail status,
- detailed violation bullets,
- line-level hints inferred from code and diagnostics,
- raw compiler output for debugging,
- CodeCarbon metrics (kWh and emissions) per phase and total.

## Operational Constraints

For successful execution, the environment must provide:

- clang,
- opt,
- built pass plugin (.so),
- Python dependencies for web-demo (Flask, CodeCarbon).

If tooling is missing, the backend returns setup-error and displays it in the UI.

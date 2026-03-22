# Week 11 Deliverable - Bug-Fix Documentation

## Scope

This document records validation-phase defects discovered while stabilizing compile-time OTA enforcement behavior.

## Bug Log

### BF-11-01: Secure rollback false positive

- Component: llvm-pass/TraversalPass.cpp
- Symptom: secure.c failed with rollback guard violation despite safe logic.
- Root cause:
  - Initial rollback checker was too strict on accepted compare/branch forms.
  - Operand provenance in some IR forms was insufficiently tracked.
- Fix applied:
  - Expanded rollback pattern support to equivalent safe forms.
  - Improved provenance tracking in compare analysis for package-derived values.
- Verification:
  - secure.c passes.
  - secure_rule_rollback.c passes.
  - insecure_rule_rollback_missing.c fails as expected.
- Status: closed.

### BF-11-02: Build failure due to rollback function corruption

- Component: llvm-pass/TraversalPass.cpp
- Symptom: C++ compilation errors (undeclared identifiers, brace mismatch, parser errors near plugin entrypoint).
- Root cause: malformed edit merge inside rollback function.
- Fix applied:
  - Replaced the entire function with a clean and structurally valid implementation.
- Verification:
  - Plugin rebuild succeeds in Ubuntu environment.
- Status: closed.

### BF-11-03: Excessive stack dump output on expected insecure failures

- Component: llvm-pass/TraversalPass.cpp
- Symptom: Insecure tests failed as expected but printed large LLVM backtraces, reducing usability.
- Root cause: fatal error reporting mode emitted crash diagnostics.
- Fix applied:
  - Updated fatal error call to disable crash diagnostic dump behavior for policy violations.
- Verification:
  - Insecure tests still fail.
  - Output is cleaner and policy-focused.
- Status: closed.

## Regression Verification

Regression checks used:

1. ./secure-clang tests/secure.c -o secure.out
2. ./secure-clang tests/insecure.c -o insecure.out
3. ./scripts/run_policy_matrix.sh

Observed outcome:
- Baseline matrix passed with 12/12 expected outcomes.
- Test suite has now been expanded to 20 total test files.
- A rerun is required to stamp final regression evidence for the expanded set.

## Open Defects

- None observed in Week 11 matrix execution.

## Follow-Up Hardening Tasks

1. Add indirect call handling for deeper robustness.
2. Expand wrapper function tracing for API matching.
3. Increase rollback matcher coverage for additional control-flow patterns.

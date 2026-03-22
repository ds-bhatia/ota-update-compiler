# Secure OTA Update Compiler - Weekly Execution Plan

## Week 1 - Policy Enforcement Core (Completed)

- Implemented hard-fail security enforcement in the LLVM pass for updateFirmware().
- Added dominance checks to require verifySignature() before install().
- Added dominance checks to require sourceTrusted() before install().
- Added rollback guard detection before install() by checking for (new_version > current_version) style comparisons.
- Added ban checks for sensitive logging APIs inside updateFirmware().
- Added ban checks for weak APIs (MD5/SHA1/rand/srand) inside updateFirmware().

## Week 2 - Realistic Firmware Test Programs (Completed)

- Rewrote tests/secure.c to model realistic OTA metadata, device state, validation stages, and status handling.
- Rewrote tests/insecure.c to model realistic but intentionally unsafe update behavior.
- Ensured insecure sample violates multiple rules to exercise pass diagnostics.

## Week 3 - Expanded Violation Test Matrix (Completed)

- Add one focused insecure test per rule:
  - Missing verifySignature dominance.
  - Missing sourceTrusted dominance.
  - Missing rollback guard.
  - Logging in updateFirmware().
  - Weak crypto API use.
- Add one secure variant per rule category to avoid false positives.

Added files:

- secure_rule_signature.c
- insecure_rule_signature_missing.c
- secure_rule_source.c
- insecure_rule_source_missing.c
- secure_rule_rollback.c
- insecure_rule_rollback_missing.c
- secure_rule_logging.c
- insecure_rule_logging.c
- secure_rule_weak_crypto.c
- insecure_rule_weak_crypto.c

## Week 4 - Precision Improvements (Completed)

- Improve rollback guard detection to ensure comparison result influences control-flow guarding install().
- Handle equivalent trusted/verify function names via configurable allowlists.
- Improve diagnostic messages with source locations and callsite context.

## Week 5 - Build and Demo Packaging (Completed)

- Add complete build/run instructions for LLVM pass plugin usage in README.
- Add reproducible commands for running secure and insecure checks.
- Optionally add script wrappers for quick demo runs.

Implemented in this week:

- README expanded with prerequisites, build commands, single-file run flow, and one-command matrix run.
- Added scripts/run_policy_matrix.ps1 for automated secure/insecure expectation checks.

## Week 6 - Final Validation and Report (In Progress)

- Run full pass over all test cases and record expected/actual results.
- Document limitations and known bypasses.
- Finalize project report with policy coverage and threat model mapping.

Implemented so far:

- Added FINAL_VALIDATION_REPORT.md with expected matrix outcomes and execution checklist.

Remaining to close Week 6:

- Execute full matrix on a machine with cmake, clang, and opt available on PATH.
- Record actual pass/fail evidence in FINAL_VALIDATION_REPORT.md.

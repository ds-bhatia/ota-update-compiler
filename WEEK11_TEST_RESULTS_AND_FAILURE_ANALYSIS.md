# Week 11 Deliverable - Test Results and Failure Analysis

## Executed Command

- ./scripts/run_policy_matrix.sh

## Observed Result Snapshot

### Executed Snapshot (Before Suite Expansion)

- Plugin used: llvm-pass/build/libTraversalPass.so
- Checked files: 12
- Result: Matrix result: PASSED

Per-file outcomes observed:

1. insecure.c: expected fail, got fail
2. insecure_rule_logging.c: expected fail, got fail
3. insecure_rule_rollback_missing.c: expected fail, got fail
4. insecure_rule_signature_missing.c: expected fail, got fail
5. insecure_rule_source_missing.c: expected fail, got fail
6. insecure_rule_weak_crypto.c: expected fail, got fail
7. secure.c: expected pass, got pass
8. secure_rule_logging.c: expected pass, got pass
9. secure_rule_rollback.c: expected pass, got pass
10. secure_rule_signature.c: expected pass, got pass
11. secure_rule_source.c: expected pass, got pass
12. secure_rule_weak_crypto.c: expected pass, got pass

## Quantitative Summary

- Executed tests: 12
- Matched expectations: 12
- Mismatches: 0
- Success rate: 100%

### Expanded Suite Status (Current)

- Current suite size: 20 tests
- Additional tests added after the snapshot: 8
- Execution status for expanded set: pending rerun

Additional tests added:

1. secure_rule_aliases.c
2. insecure_rule_aliases_missing_verify.c
3. secure_rule_rollback_alt_form.c
4. insecure_rule_rollback_equal_allowed.c
5. secure_rule_no_install_path.c
6. insecure_rule_logging_puts.c
7. secure_rule_multistage_checks.c
8. insecure_rule_weak_md5.c

## Failure Analysis

Current matrix run:
- No unexpected failures.
- No false positives in secure tests.
- No false negatives in insecure tests.

Expanded suite note:
- Final failure analysis for the newly added 8 tests will be confirmed after rerunning ./scripts/run_policy_matrix.sh.

Historical defects discovered during validation:

1. False positive on secure rollback pattern
- Symptom: secure.c was flagged for missing rollback guard.
- Root cause: rollback matcher initially accepted only limited branch forms and did not robustly follow some IR forms.
- Resolution: matcher updated to support equivalent safe branch shapes and improved provenance handling.
- Status: resolved.

2. Build break in rollback function
- Symptom: compile errors in TraversalPass.cpp for undeclared symbols and mismatched braces.
- Root cause: corrupted function merge during iterative edits.
- Resolution: rollback function replaced with a clean, validated implementation.
- Status: resolved.

3. Noisy LLVM crash backtraces on expected policy violations
- Symptom: insecure tests produced long stack dumps after expected fail.
- Root cause: fatal error path emitted crash diagnostics.
- Resolution: fatal reporting configured for cleaner user-facing failure output.
- Status: resolved.

## Remaining Technical Risks

- Indirect/function-pointer call analysis is limited.
- Name-based API matching can be bypassed through wrappers if not expanded via deeper call graph analysis.
- Complex control-flow forms may still require additional rollback pattern support in future hardening.

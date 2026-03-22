# Week 11 Deliverable - Test Case Suite

## Objective

Validate compile-time enforcement for OTA security invariants in updateFirmware() using secure and insecure firmware programs.

## Test Execution Method

Run all tests through the matrix runner:

1. Build plugin:
   - cmake -S llvm-pass -B llvm-pass/build
   - cmake --build llvm-pass/build
2. Run matrix:
   - ./scripts/run_policy_matrix.sh

The matrix runner behavior is defined in scripts/run_policy_matrix.sh:
- secure* files are expected to pass.
- insecure* files are expected to fail.

## Test Inventory

### End-to-End Baseline Tests

1. tests/secure.c
- Purpose: Full secure OTA flow.
- Expected: pass.
- Covered invariants: signature check, trusted source check, rollback guard, no weak API use in updateFirmware, no sensitive logging in updateFirmware.

2. tests/insecure.c
- Purpose: Full insecure OTA flow with multiple violations.
- Expected: fail.
- Covered invariants: logging leakage, weak API usage, missing pre-install dominance checks, missing rollback gating.

### Rule-Focused Tests

3. tests/secure_rule_signature.c
- Purpose: Signature dominance positive case.
- Expected: pass.

4. tests/insecure_rule_signature_missing.c
- Purpose: Signature dominance negative case.
- Expected: fail.

5. tests/secure_rule_source.c
- Purpose: Trusted source dominance positive case.
- Expected: pass.

6. tests/insecure_rule_source_missing.c
- Purpose: Trusted source dominance negative case.
- Expected: fail.

7. tests/secure_rule_rollback.c
- Purpose: Rollback guard positive case.
- Expected: pass.

8. tests/insecure_rule_rollback_missing.c
- Purpose: Rollback guard negative case.
- Expected: fail.

9. tests/secure_rule_logging.c
- Purpose: No logging leakage inside updateFirmware positive case.
- Expected: pass.

10. tests/insecure_rule_logging.c
- Purpose: Logging leakage negative case.
- Expected: fail.

11. tests/secure_rule_weak_crypto.c
- Purpose: Weak API avoidance positive case.
- Expected: pass.

12. tests/insecure_rule_weak_crypto.c
- Purpose: Weak API usage negative case.
- Expected: fail.

13. tests/secure_rule_aliases.c
- Purpose: Verify alias allowlist support (checkSignature/validateSource/applyUpdate).
- Expected: pass.

14. tests/insecure_rule_aliases_missing_verify.c
- Purpose: Alias-based install path without pre-install signature dominance.
- Expected: fail.

15. tests/secure_rule_rollback_alt_form.c
- Purpose: Valid rollback guard in alternate equivalent form (current_version >= pkg->version -> reject).
- Expected: pass.

16. tests/insecure_rule_rollback_equal_allowed.c
- Purpose: Incorrect rollback guard that allows equal version installation.
- Expected: fail.

17. tests/secure_rule_no_install_path.c
- Purpose: No-install control path should not be falsely flagged.
- Expected: pass.

18. tests/insecure_rule_logging_puts.c
- Purpose: Logging leakage variant using puts inside updateFirmware.
- Expected: fail.

19. tests/secure_rule_multistage_checks.c
- Purpose: Secure multi-stage validation flow before install.
- Expected: pass.

20. tests/insecure_rule_weak_md5.c
- Purpose: Weak crypto variant using MD5 inside updateFirmware.
- Expected: fail.

## Coverage Summary

- Total matrix tests: 20
- Positive tests: 10
- Negative tests: 10
- Invariant categories covered: 7

## Pass Criteria

- Every secure* test passes.
- Every insecure* test fails.
- Matrix exits with status 0 and prints Matrix result: PASSED.

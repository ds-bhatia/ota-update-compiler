/*
 * TraversalPass.cpp — OTA Security Enforcement LLVM Pass
 *
 * This LLVM New Pass Manager plugin statically analyzes the
 * updateFirmware() function in firmware C code and enforces
 * the following security invariants:
 *
 *   Rule 1: verifySignature() must dominate install()
 *   Rule 2: Version rollback check (icmp on current_version) must
 *           dominate install()
 *   Rule 3: sourceTrusted() must dominate install()
 *   Rule 4: install() must only be reachable via conditional branches
 *
 * Violations are reported to stderr and written to secure_log.txt.
 */

#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <fstream>
#include <string>
#include <vector>
#include <ctime>

using namespace llvm;

namespace {

/* ------------------------------------------------------------------ */
/*  Result entry for the security audit log                            */
/* ------------------------------------------------------------------ */
struct RuleResult {
    std::string rule_id;
    std::string description;
    bool        passed;
    std::string detail;
};

/* ------------------------------------------------------------------ */
/*  OTASecurityPass                                                    */
/* ------------------------------------------------------------------ */
class OTASecurityPass : public PassInfoMixin<OTASecurityPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {

        /* Only analyze the update entry point */
        if (F.getName() != "updateFirmware")
            return PreservedAnalyses::all();

        errs() << "\n";
        errs() << "========================================================\n";
        errs() << "  OTA Security Compiler — Static Analysis Report\n";
        errs() << "========================================================\n";
        errs() << "  Function : " << F.getName() << "\n";
        errs() << "  Module   : " << F.getParent()->getSourceFileName() << "\n";
        errs() << "========================================================\n\n";

        std::vector<RuleResult> results;

        /* ----- Phase 1: CFG Structure ----- */
        errs() << "--- Control Flow Graph ---\n";
        for (auto &BB : F) {
            errs() << "  BasicBlock: " << BB.getName() << "\n";
            for (auto *Succ : successors(&BB)) {
                errs() << "    -> " << Succ->getName() << "\n";
            }
        }
        errs() << "\n";

        /* ----- Phase 2: Locate install() call site ----- */
        BasicBlock *InstallBlock = nullptr;
        CallInst   *InstallCall  = nullptr;

        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (Function *Callee = CI->getCalledFunction()) {
                        if (Callee->getName() == "install") {
                            InstallBlock = &BB;
                            InstallCall  = CI;
                            errs() << "[CFG] install() found in block: "
                                   << BB.getName() << "\n";
                        }
                    }
                }
            }
        }

        if (!InstallBlock) {
            errs() << "[INFO] No install() call found — nothing to audit.\n";
            return PreservedAnalyses::all();
        }

        /* ----- Phase 3: Dominator Tree ----- */
        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

        errs() << "\n--- Dominance Info ---\n";
        for (auto &BB : F) {
            if (DT.dominates(&BB, InstallBlock)) {
                errs() << "  Block " << BB.getName()
                       << " dominates install block ("
                       << InstallBlock->getName() << ")\n";
            }
        }
        errs() << "\n";

        /* ----- Phase 4: Collect security-relevant calls in  ----- */
        /*                 blocks that dominate install()            */

        bool hasSignatureCheck = false;
        bool hasSourceCheck    = false;
        bool hasVersionCheck   = false;

        std::string sigBlock, srcBlock, verBlock;

        for (auto &BB : F) {
            if (!DT.dominates(&BB, InstallBlock))
                continue;

            for (auto &I : BB) {
                /* Check for security function calls */
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (Function *Callee = CI->getCalledFunction()) {
                        StringRef name = Callee->getName();

                        if (name == "verifySignature") {
                            hasSignatureCheck = true;
                            sigBlock = BB.getName().str();
                        }
                        if (name == "sourceTrusted") {
                            hasSourceCheck = true;
                            srcBlock = BB.getName().str();
                        }
                    }
                }

                /* Check for version comparison (icmp involving
                   current_version global) */
                if (auto *Cmp = dyn_cast<ICmpInst>(&I)) {
                    for (unsigned op = 0; op < Cmp->getNumOperands(); ++op) {
                        Value *V = Cmp->getOperand(op);

                        /* Walk through load → GEP → global chain */
                        if (auto *Load = dyn_cast<LoadInst>(V)) {
                            Value *Ptr = Load->getPointerOperand();
                            /* Direct global reference */
                            if (auto *GV = dyn_cast<GlobalVariable>(Ptr)) {
                                if (GV->getName().contains("current_version")) {
                                    hasVersionCheck = true;
                                    verBlock = BB.getName().str();
                                }
                            }
                            /* GEP into device_config struct */
                            if (auto *GEP = dyn_cast<GetElementPtrInst>(Ptr)) {
                                Value *Base = GEP->getPointerOperand();
                                if (auto *GV = dyn_cast<GlobalVariable>(Base)) {
                                    if (GV->getName().contains("device_config")) {
                                        hasVersionCheck = true;
                                        verBlock = BB.getName().str();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        /* ----- Phase 5: Conditional guard check ----- */
        /*  install() should NOT be reachable via an unconditional
         *  branch from the function entry.  We verify that every
         *  predecessor of install's block uses a conditional branch. */

        bool hasConditionalGuard = true;
        std::string uncondPredName;

        for (auto *Pred : predecessors(InstallBlock)) {
            Instruction *Term = Pred->getTerminator();
            if (auto *Br = dyn_cast<BranchInst>(Term)) {
                if (Br->isUnconditional()) {
                    hasConditionalGuard = false;
                    uncondPredName = Pred->getName().str();
                }
            }
        }

        /* Also check if install block IS the entry block */
        if (InstallBlock == &F.getEntryBlock()) {
            hasConditionalGuard = false;
            uncondPredName = "(entry)";
        }

        /* ----- Phase 6: Build results ----- */

        results.push_back({
            "RULE-1", "Signature verification (verifySignature dominates install)",
            hasSignatureCheck,
            hasSignatureCheck
                ? "verifySignature() found in dominating block [" + sigBlock + "]"
                : "verifySignature() NOT found on any path dominating install()"
        });

        results.push_back({
            "RULE-2", "Rollback prevention (version comparison dominates install)",
            hasVersionCheck,
            hasVersionCheck
                ? "Version comparison found in dominating block [" + verBlock + "]"
                : "No version/rollback check found dominating install()"
        });

        results.push_back({
            "RULE-3", "Source trust validation (sourceTrusted dominates install)",
            hasSourceCheck,
            hasSourceCheck
                ? "sourceTrusted() found in dominating block [" + srcBlock + "]"
                : "sourceTrusted() NOT found on any path dominating install()"
        });

        results.push_back({
            "RULE-4", "Conditional guard (install reachable only via conditional branch)",
            hasConditionalGuard,
            hasConditionalGuard
                ? "All predecessors of install block use conditional branches"
                : "Unconditional branch reaches install from block [" + uncondPredName + "]"
        });

        /* ----- Phase 7: Print report ----- */

        int passed = 0, failed = 0;

        errs() << "--- Security Rule Enforcement ---\n\n";
        for (auto &R : results) {
            if (R.passed) {
                errs() << "  [PASS] " << R.rule_id << ": "
                       << R.description << "\n"
                       << "         " << R.detail << "\n\n";
                passed++;
            } else {
                errs() << "  [FAIL] " << R.rule_id << ": "
                       << R.description << "\n"
                       << "         " << R.detail << "\n\n";
                failed++;
            }
        }

        errs() << "========================================================\n";
        if (failed == 0) {
            errs() << "  RESULT: ALL CHECKS PASSED (" << passed << "/"
                   << passed << ")\n";
            errs() << "  Firmware update code is SECURE.\n";
        } else {
            errs() << "  RESULT: " << failed << " VIOLATION(S) DETECTED ("
                   << passed << "/" << (passed + failed) << " passed)\n";
            errs() << "  Firmware update code is INSECURE.\n";
        }
        errs() << "========================================================\n\n";

        /* ----- Phase 8: Write log file ----- */
        writeLog(F, results, passed, failed);

        return PreservedAnalyses::all();
    }

private:
    void writeLog(Function &F,
                  const std::vector<RuleResult> &results,
                  int passed, int failed) {

        std::string filename = "secure_log.txt";
        std::ofstream log(filename, std::ios::trunc);
        if (!log.is_open()) {
            errs() << "[WARN] Could not open " << filename << " for writing\n";
            return;
        }

        time_t now = time(nullptr);
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S",
                 localtime(&now));

        log << "============================================================\n";
        log << "  OTA Security Compiler — Audit Log\n";
        log << "============================================================\n";
        log << "  Timestamp : " << timebuf << "\n";
        log << "  Function  : " << F.getName().str() << "\n";
        log << "  Module    : " << F.getParent()->getSourceFileName() << "\n";
        log << "============================================================\n\n";

        for (auto &R : results) {
            log << (R.passed ? "[PASS] " : "[FAIL] ")
                << R.rule_id << ": " << R.description << "\n"
                << "       " << R.detail << "\n\n";
        }

        log << "============================================================\n";
        if (failed == 0) {
            log << "  VERDICT: SECURE (" << passed << "/"
                << passed << " rules passed)\n";
        } else {
            log << "  VERDICT: INSECURE (" << failed
                << " violation(s), " << passed << "/"
                << (passed + failed) << " passed)\n";
        }
        log << "============================================================\n";

        log.close();
        errs() << "[LOG] Report written to " << filename << "\n";
    }
};

} // end anonymous namespace

/* ------------------------------------------------------------------ */
/*  Pass Plugin Registration                                           */
/* ------------------------------------------------------------------ */

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "OTASecurityPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "ota-security") {
                        FPM.addPass(OTASecurityPass());
                        return true;
                    }
                    /* Keep backward compat with old name */
                    if (Name == "traversal-pass") {
                        FPM.addPass(OTASecurityPass());
                        return true;
                    }
                    return false;
                });
        }};
}


#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

#include <queue>
#include <string>
#include <vector>

using namespace llvm;

namespace {

static bool isInList(StringRef Name, std::initializer_list<StringRef> Names) {
    for (StringRef N : Names) {
        if (Name == N) {
            return true;
        }
    }
    return false;
}

static bool isNameMatch(StringRef Name,
                        std::initializer_list<StringRef> AllowedNames) {
    for (StringRef N : AllowedNames) {
        if (Name == N) {
            return true;
        }
    }
    return false;
}

static std::string blockNameOrFallback(const BasicBlock *BB) {
    if (!BB) {
        return "<null-bb>";
    }

    if (BB->hasName()) {
        return BB->getName().str();
    }

    return "<unnamed-bb>";
}

static std::string instructionSite(const Instruction *I) {
    std::string S;
    raw_string_ostream OS(S);

    OS << "bb=" << blockNameOrFallback(I ? I->getParent() : nullptr) << " | inst=";
    if (I) {
        I->print(OS);
    } else {
        OS << "<null-inst>";
    }
    return OS.str();
}

static bool blockReachesTarget(BasicBlock *Start, BasicBlock *Target) {
    if (!Start || !Target) {
        return false;
    }

    if (Start == Target) {
        return true;
    }

    SmallPtrSet<BasicBlock *, 64> Visited;
    std::queue<BasicBlock *> Q;
    Visited.insert(Start);
    Q.push(Start);

    while (!Q.empty()) {
        BasicBlock *BB = Q.front();
        Q.pop();

        for (BasicBlock *Succ : successors(BB)) {
            if (Succ == Target) {
                return true;
            }
            if (Visited.insert(Succ).second) {
                Q.push(Succ);
            }
        }
    }

    return false;
}

static bool valueDerivedFrom(Value *V, const Value *Target,
                             SmallPtrSetImpl<const Value *> &Visited) {
    if (!V) {
        return false;
    }

    V = V->stripPointerCasts();

    if (V == Target) {
        return true;
    }

    if (!Visited.insert(V).second) {
        return false;
    }

    if (auto *LI = dyn_cast<LoadInst>(V)) {
        return valueDerivedFrom(LI->getPointerOperand(), Target, Visited);
    }

    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
        return valueDerivedFrom(GEP->getPointerOperand(), Target, Visited);
    }

    if (auto *Cast = dyn_cast<CastInst>(V)) {
        return valueDerivedFrom(Cast->getOperand(0), Target, Visited);
    }

    if (auto *PHI = dyn_cast<PHINode>(V)) {
        for (Value *Incoming : PHI->incoming_values()) {
            if (valueDerivedFrom(Incoming, Target, Visited)) {
                return true;
            }
        }
        return false;
    }

    if (auto *Sel = dyn_cast<SelectInst>(V)) {
        return valueDerivedFrom(Sel->getTrueValue(), Target, Visited) ||
               valueDerivedFrom(Sel->getFalseValue(), Target, Visited);
    }

    return false;
}

static bool isPkgDerived(Value *V, Argument *PkgArg) {
    SmallPtrSet<const Value *, 64> Visited;
    return valueDerivedFrom(V, PkgArg, Visited);
}

static bool isCurrentVersionDerivedImpl(Value *V,
                                        SmallPtrSetImpl<const Value *> &Visited) {

    if (!V) {
        return false;
    }

    V = V->stripPointerCasts();

    if (auto *GV = dyn_cast<GlobalVariable>(V)) {
        return GV->getName() == "current_version";
    }

    if (!Visited.insert(V).second) {
        return false;
    }

    if (auto *LI = dyn_cast<LoadInst>(V)) {
        return isCurrentVersionDerivedImpl(LI->getPointerOperand(), Visited);
    }

    if (auto *GEP = dyn_cast<GetElementPtrInst>(V)) {
        return isCurrentVersionDerivedImpl(GEP->getPointerOperand(), Visited);
    }

    if (auto *Cast = dyn_cast<CastInst>(V)) {
        return isCurrentVersionDerivedImpl(Cast->getOperand(0), Visited);
    }

    if (auto *PHI = dyn_cast<PHINode>(V)) {
        for (Value *Incoming : PHI->incoming_values()) {
            if (isCurrentVersionDerivedImpl(Incoming, Visited)) {
                return true;
            }
        }
        return false;
    }

    if (auto *Sel = dyn_cast<SelectInst>(V)) {
        return isCurrentVersionDerivedImpl(Sel->getTrueValue(), Visited) ||
               isCurrentVersionDerivedImpl(Sel->getFalseValue(), Visited);
    }

    return false;
}

static bool isCurrentVersionDerived(Value *V) {
    SmallPtrSet<const Value *, 64> Visited;
    return isCurrentVersionDerivedImpl(V, Visited);
}

static bool hasRollbackGuardBeforeInstall(Function &F, DominatorTree &DT,
                                          Instruction *InstallI,
                                          Argument *PkgArg) {
    BasicBlock *InstallBB = InstallI ? InstallI->getParent() : nullptr;

    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            auto *Cmp = dyn_cast<ICmpInst>(&I);
            if (!Cmp) {
                continue;
            }

            if (!DT.dominates(Cmp, InstallI)) {
                continue;
            }

            Value *LHS = Cmp->getOperand(0);
            Value *RHS = Cmp->getOperand(1);

            bool PkgL = isPkgDerived(LHS, PkgArg);
            bool CurR = isCurrentVersionDerived(RHS);
            bool CurL = isCurrentVersionDerived(LHS);
            bool PkgR = isPkgDerived(RHS, PkgArg);

            ICmpInst::Predicate Pred = Cmp->getPredicate();

            if ((Pred == ICmpInst::ICMP_SGT || Pred == ICmpInst::ICMP_UGT) &&
                PkgL && CurR) {
                User *SingleUser = Cmp->hasOneUse() ? *Cmp->user_begin() : nullptr;
                auto *Br = dyn_cast_or_null<BranchInst>(SingleUser);
                if (!Br || !Br->isConditional() || Br->getCondition() != Cmp) {
                    continue;
                }

                BasicBlock *TrueSucc = Br->getSuccessor(0);
                BasicBlock *FalseSucc = Br->getSuccessor(1);
                bool TrueReachesInstall = blockReachesTarget(TrueSucc, InstallBB);
                bool FalseReachesInstall = blockReachesTarget(FalseSucc, InstallBB);

                if (TrueReachesInstall && !FalseReachesInstall) {
                    return true;
                }
            }

            if ((Pred == ICmpInst::ICMP_SLT || Pred == ICmpInst::ICMP_ULT) &&
                CurL && PkgR) {
                User *SingleUser = Cmp->hasOneUse() ? *Cmp->user_begin() : nullptr;
                auto *Br = dyn_cast_or_null<BranchInst>(SingleUser);
                if (!Br || !Br->isConditional() || Br->getCondition() != Cmp) {
                    continue;
                }

                BasicBlock *TrueSucc = Br->getSuccessor(0);
                BasicBlock *FalseSucc = Br->getSuccessor(1);
                bool TrueReachesInstall = blockReachesTarget(TrueSucc, InstallBB);
                bool FalseReachesInstall = blockReachesTarget(FalseSucc, InstallBB);

                if (TrueReachesInstall && !FalseReachesInstall) {
                    return true;
                }
            }
        }
    }

    return false;
}

class TraversalPass : public PassInfoMixin<TraversalPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {

        if (F.getName() != "updateFirmware")
            return PreservedAnalyses::all();

        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

        std::vector<CallInst *> InstallCalls;
        std::vector<CallInst *> VerifyCalls;
        std::vector<CallInst *> TrustedSourceCalls;
        std::vector<std::string> Violations;

        const std::initializer_list<StringRef> InstallNames = {
            "install", "installFirmware", "applyUpdate"
        };
        const std::initializer_list<StringRef> VerifyNames = {
            "verifySignature", "verify_signature", "checkSignature"
        };
        const std::initializer_list<StringRef> TrustedSourceNames = {
            "sourceTrusted", "isSourceTrusted", "validateSource"
        };

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                auto *CI = dyn_cast<CallInst>(&I);
                if (!CI) {
                    continue;
                }

                Function *Callee = CI->getCalledFunction();
                if (!Callee) {
                    continue;
                }

                StringRef Name = Callee->getName();

                if (isNameMatch(Name, InstallNames)) {
                    InstallCalls.push_back(CI);
                    continue;
                }

                if (isNameMatch(Name, VerifyNames)) {
                    VerifyCalls.push_back(CI);
                    continue;
                }

                if (isNameMatch(Name, TrustedSourceNames)) {
                    TrustedSourceCalls.push_back(CI);
                    continue;
                }

                if (isInList(Name,
                             {"printf", "puts", "fprintf", "perror", "syslog", "vsyslog", "snprintf"})) {
                    Violations.push_back("Sensitive logging API call inside updateFirmware(): " +
                                         Name.str() + " at " + instructionSite(CI));
                    continue;
                }

                if (isInList(Name,
                             {"MD5", "MD5_Init", "MD5_Update", "MD5_Final", "SHA1", "SHA1_Init", "SHA1_Update", "SHA1_Final", "rand", "srand"})) {
                    Violations.push_back("Weak crypto or weak entropy API inside updateFirmware(): " +
                                         Name.str() + " at " + instructionSite(CI));
                }
            }
        }

        for (CallInst *InstallCI : InstallCalls) {
            bool VerifyDominates = false;
            for (CallInst *VerifyCI : VerifyCalls) {
                if (DT.dominates(VerifyCI, InstallCI)) {
                    VerifyDominates = true;
                    break;
                }
            }
            if (!VerifyDominates) {
                Violations.push_back("Install call is not dominated by signature verification on all paths at " +
                                     instructionSite(InstallCI));
            }

            bool SourceDominates = false;
            for (CallInst *TrustedCI : TrustedSourceCalls) {
                if (DT.dominates(TrustedCI, InstallCI)) {
                    SourceDominates = true;
                    break;
                }
            }
            if (!SourceDominates) {
                Violations.push_back("Install call is not dominated by trusted source validation on all paths at " +
                                     instructionSite(InstallCI));
            }

            Argument *PkgArg = F.arg_size() > 0 ? F.getArg(0) : nullptr;
            if (!PkgArg || !hasRollbackGuardBeforeInstall(F, DT, InstallCI, PkgArg)) {
                Violations.push_back("Rollback guard '(new_version > current_version)' does not gate install path at " +
                                     instructionSite(InstallCI));
            }
        }

        if (!Violations.empty()) {
            std::string Message =
                "[OTA Security Pass] Security policy violation(s) in updateFirmware():\n";
            for (const std::string &V : Violations) {
                Message += " - " + V + "\n";
            }
            report_fatal_error(Message);
        }

        return PreservedAnalyses::all();
    }
};

} 

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "TraversalPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "traversal-pass") {
                        FPM.addPass(TraversalPass());
                        return true;
                    }
                    return false;
                });
        }};
}


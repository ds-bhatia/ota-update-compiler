#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

namespace {

class TraversalPass : public PassInfoMixin<TraversalPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {

        // Only analyze updateFirmware
        if (F.getName() != "updateFirmware")
            return PreservedAnalyses::all();

        errs() << "===== Analyzing Function: " << F.getName() << " =====\n";

        // ---------------------------
        // 1️⃣ Print CFG Structure
        // ---------------------------
        for (auto &BB : F) {
            errs() << "BasicBlock: " << BB.getName() << "\n";

            for (auto *Succ : successors(&BB)) {
                errs() << "  -> " << Succ->getName() << "\n";
            }
        }

        // ---------------------------
        // 2️⃣ Find install() block
        // ---------------------------
        BasicBlock *InstallBlock = nullptr;

        for (auto &BB : F) {
            for (auto &I : BB) {
                if (auto *CI = dyn_cast<CallInst>(&I)) {
                    if (Function *Callee = CI->getCalledFunction()) {
                        if (Callee->getName() == "install") {
                            InstallBlock = &BB;
                            errs() << "[CFG] install() found in block: "
                                   << BB.getName() << "\n";
                        }
                    }
                }
            }
        }

        // ---------------------------
        // 3️⃣ Dominator Analysis
        // ---------------------------
        if (InstallBlock) {
            DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);

            errs() << "\n[Dominance Info]\n";
            for (auto &BB : F) {
                if (DT.dominates(&BB, InstallBlock)) {
                    errs() << "Block " << BB.getName()
                           << " dominates install block\n";
                }
            }
        }

        errs() << "===== End Analysis =====\n\n";

        return PreservedAnalyses::all();
    }
};

} // namespace

// Register Pass
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


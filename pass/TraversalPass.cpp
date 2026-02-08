#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct TraversalPass : public PassInfoMixin<TraversalPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
  errs() << "[TraversalPass] Visiting function: " << F.getName() << "\n";


    // We only care about updateFirmware
   // if (F.getName() != "updateFirmware")
     // return PreservedAnalyses::all();

    errs() << "\n[TraversalPass] Entering function: "
           << F.getName() << "\n";

    // Walk through every basic block
    for (BasicBlock &BB : F) {
      // Walk through every instruction
      for (Instruction &I : BB) {

        // Check if instruction is a function call
        auto *call = dyn_cast<CallBase>(&I);
        if (!call)
          continue;

        Function *callee = call->getCalledFunction();
        if (!callee)
          continue;

        errs() << "  Found function call: "
               << callee->getName() << "\n";
      }
    }

    return PreservedAnalyses::all();
  }
};

} // end anonymous namespace

// Plugin registration
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return {
      LLVM_PLUGIN_API_VERSION,
      "TraversalPass",
      LLVM_VERSION_STRING,
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


#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

class UpdateVisitor : public RecursiveASTVisitor<UpdateVisitor> {
public:
  explicit UpdateVisitor(ASTContext *Context) : Context(Context) {}

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody())
      return true;

    if (FD->getNameAsString() == "updateFirmware") {
      llvm::errs() << "[AST] Found updateFirmware()\n";

      for (auto *Stmt : FD->getBody()->children()) {
        Stmt->dump(); // optional debug
      }
    }
    return true;
  }

  bool VisitCallExpr(CallExpr *CE) {
    if (FunctionDecl *Callee = CE->getDirectCallee()) {
      llvm::errs() << "  [AST] Function call: "
                   << Callee->getNameAsString() << "\n";
    }
    return true;
  }

private:
  ASTContext *Context;
};

class UpdateASTConsumer : public ASTConsumer {
public:
  explicit UpdateASTConsumer(ASTContext *Context)
      : Visitor(Context) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  UpdateVisitor Visitor;
};

class UpdateASTAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<UpdateASTConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // namespace

static FrontendPluginRegistry::Add<UpdateASTAction>
X("update-ast", "Detect firmware update logic");

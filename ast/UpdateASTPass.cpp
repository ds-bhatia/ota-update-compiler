/*
 * UpdateASTPass.cpp — Clang AST Frontend Plugin for OTA Security
 *
 * This plugin runs at the AST (source) level before LLVM IR is
 * generated.  It performs the following:
 *
 *   1. Locates the updateFirmware() function definition
 *   2. Enumerates all security-relevant function calls inside it
 *   3. Checks whether verifySignature / sourceTrusted / install
 *      are invoked, and in what order
 *   4. Detects if install() is called outside an if-guard
 *   5. Reports source locations for every finding
 *
 * Build:
 *   clang++ -shared -fPIC -fno-rtti \
 *     $(llvm-config --cxxflags) \
 *     UpdateASTPass.cpp -o UpdateASTPass.so \
 *     $(llvm-config --ldflags)
 *
 * Run:
 *   clang -cc1 -load ./UpdateASTPass.so -plugin update-ast <file>.c
 */

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <string>
#include <vector>

using namespace clang;

namespace {

/* ------------------------------------------------------------------ */
/*  AST Visitor                                                        */
/* ------------------------------------------------------------------ */

class OTAUpdateVisitor : public RecursiveASTVisitor<OTAUpdateVisitor> {
public:
  explicit OTAUpdateVisitor(ASTContext *Ctx) : Context(Ctx) {}

  /* ---- Visit every function definition ---- */
  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (!FD->hasBody())
      return true;

    std::string fname = FD->getNameAsString();
    SourceLocation Loc = FD->getBeginLoc();
    std::string locStr = Loc.printToString(Context->getSourceManager());

    if (fname == "updateFirmware") {
      llvm::errs() << "\n[AST] ===== Found updateFirmware() at "
                   << locStr << " =====\n";
      InsideUpdateFirmware = true;
      analyzeUpdateBody(FD);
      InsideUpdateFirmware = false;
    }

    return true;
  }

  /* ---- Visit every call expression ---- */
  bool VisitCallExpr(CallExpr *CE) {
    FunctionDecl *Callee = CE->getDirectCallee();
    if (!Callee)
      return true;

    std::string name = Callee->getNameAsString();
    SourceLocation Loc = CE->getBeginLoc();
    std::string locStr = Loc.printToString(Context->getSourceManager());

    /* Flag any security-relevant call anywhere in the TU */
    if (name == "verifySignature" || name == "sourceTrusted" ||
        name == "install" || name == "updateFirmware") {
      llvm::errs() << "[AST] Security-relevant call: " << name
                   << "  at " << locStr << "\n";
    }

    return true;
  }

private:
  ASTContext *Context;
  bool InsideUpdateFirmware = false;

  /* -------------------------------------------------------------- */
  /*  Deeper analysis of the body of updateFirmware()               */
  /* -------------------------------------------------------------- */
  void analyzeUpdateBody(FunctionDecl *FD) {
    Stmt *Body = FD->getBody();
    if (!Body)
      return;

    bool foundVerify  = false;
    bool foundSource  = false;
    bool foundInstall = false;
    bool installGuarded = false;

    /* Walk top-level statements in order */
    for (auto *Child : Body->children()) {
      scanStmt(Child, foundVerify, foundSource, foundInstall,
               installGuarded, /*depth=*/0);
    }

    /* Summary */
    llvm::errs() << "\n[AST] --- updateFirmware() Summary ---\n";
    llvm::errs() << "[AST]   verifySignature called : "
                 << (foundVerify  ? "YES" : "NO") << "\n";
    llvm::errs() << "[AST]   sourceTrusted called   : "
                 << (foundSource  ? "YES" : "NO") << "\n";
    llvm::errs() << "[AST]   install called          : "
                 << (foundInstall ? "YES" : "NO") << "\n";
    llvm::errs() << "[AST]   install inside if-guard : "
                 << (installGuarded ? "YES" : "NO") << "\n";

    if (foundInstall && !foundVerify)
      llvm::errs() << "[AST-WARN] install() called without "
                      "verifySignature()!\n";
    if (foundInstall && !foundSource)
      llvm::errs() << "[AST-WARN] install() called without "
                      "sourceTrusted()!\n";
    if (foundInstall && !installGuarded)
      llvm::errs() << "[AST-WARN] install() is NOT inside a "
                      "conditional guard!\n";

    llvm::errs() << "[AST] --- End Summary ---\n\n";
  }

  /* Recursively scan statements */
  void scanStmt(Stmt *S, bool &fV, bool &fS, bool &fI,
                bool &iGuarded, int depth) {
    if (!S)
      return;

    /* Call expression */
    if (auto *CE = dyn_cast<CallExpr>(S)) {
      if (FunctionDecl *Callee = CE->getDirectCallee()) {
        std::string n = Callee->getNameAsString();
        if (n == "verifySignature") fV = true;
        if (n == "sourceTrusted")   fS = true;
        if (n == "install") {
          fI = true;
          if (depth > 0)
            iGuarded = true;  /* inside an if/else */
        }
      }
    }

    /* If statement — increase depth for children */
    if (auto *If = dyn_cast<IfStmt>(S)) {
      if (If->getThen())
        scanStmt(If->getThen(), fV, fS, fI, iGuarded, depth + 1);
      if (If->getElse())
        scanStmt(If->getElse(), fV, fS, fI, iGuarded, depth + 1);
      return;
    }

    /* Compound statement */
    if (auto *CS = dyn_cast<CompoundStmt>(S)) {
      for (auto *Child : CS->body())
        scanStmt(Child, fV, fS, fI, iGuarded, depth);
      return;
    }

    /* Recurse into children generically */
    for (auto *Child : S->children())
      scanStmt(Child, fV, fS, fI, iGuarded, depth);
  }
};

/* ------------------------------------------------------------------ */
/*  AST Consumer                                                       */
/* ------------------------------------------------------------------ */

class OTAUpdateConsumer : public ASTConsumer {
public:
  explicit OTAUpdateConsumer(ASTContext *Context)
      : Visitor(Context) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    Visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  OTAUpdateVisitor Visitor;
};

/* ------------------------------------------------------------------ */
/*  Plugin Action                                                      */
/* ------------------------------------------------------------------ */

class OTAUpdateAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<OTAUpdateConsumer>(&CI.getASTContext());
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // end anonymous namespace

static FrontendPluginRegistry::Add<OTAUpdateAction>
    X("update-ast", "OTA firmware update security analysis (AST level)");

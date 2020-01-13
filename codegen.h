#pragma once
#include "parser.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

/*
llvm::Type* typeof_expr(std::unique_ptr<AstExpr> expr);
class CodeGen {
    std::unique_ptr<llvm::Module> mod;
    CodeGen();
    void gen_decl(std::unique_ptr<FnDecl> expr);
    void gen_call(std::unique_ptr<FnCall> expr,llvm::BasicBlock* block);
};
*/

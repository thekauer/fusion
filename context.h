#pragma once
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include <memory>
#include <map>
static llvm::LLVMContext ctx;
static llvm::IRBuilder<> builder(ctx);
static std::unique_ptr<llvm::Module> mod = std::make_unique<llvm::Module>("test",ctx);
static std::map<llvm::StringRef,llvm::Value*> NamedValues;

enum class Linkage : unsigned char {
    Ext
};
enum class Inline : unsigned char {
    Alway,
    Never,
    Def
};
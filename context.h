#pragma once
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include <map>
enum class Linkage : unsigned char {
    Ext
};
enum class Inline : unsigned char {
    Alway,
    Never,
    Def
};


struct FusionCtx {
    std::vector<llvm::Value*> string_table;
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder=llvm::IRBuilder<>(ctx);
    std::unique_ptr<llvm::Module> mod = std::make_unique<llvm::Module>("global",ctx);
    std::map<llvm::StringRef,llvm::Value*> named_values;
    llvm::Type* getI8();
    llvm::Type* getI16();
    llvm::Type* getI32();
    llvm::Type* getI64();
    llvm::Type* getString();
    // llvm::Type* getU8() const;
    // llvm::Type* getU8() const;
    // llvm::Type* getU16() const;
    // llvm::Type* getU32() const;
    // llvm::Type* getU64() const;

    //llvm::Type* getIS() const;
    //llvm::Type* getUS() const;

    // llvm::Type* getBool() const;
    // llvm::Type* getChar() const;


};

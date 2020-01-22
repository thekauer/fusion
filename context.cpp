#pragma once
#include "context.h"



FusionCtx::FusionCtx(llvm::StringRef modname) :
 mod(make_unique<llvm::Module>(modname,ctx)) {};




llvm::Type* FusionCtx::getI8() {
    return llvm::IntegerType::getInt8Ty(ctx);
}
llvm::Type* FusionCtx::getI16() {
    return llvm::IntegerType::getInt16Ty(ctx);
}
llvm::Type* FusionCtx::getI32() {
    return llvm::IntegerType::getInt32Ty(ctx);
}
llvm::Type* FusionCtx::getI64() {
    return llvm::IntegerType::getInt64Ty(ctx);
}



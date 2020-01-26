#include "context.h"

llvm::Type *FusionCtx::getI8() { return llvm::IntegerType::getInt8Ty(ctx); }
llvm::Type *FusionCtx::getI16() { return llvm::IntegerType::getInt16Ty(ctx); }
llvm::Type *FusionCtx::getI32() { return llvm::IntegerType::getInt32Ty(ctx); }
llvm::Type *FusionCtx::getI64() { return llvm::IntegerType::getInt64Ty(ctx); }
llvm::Type *FusionCtx::getString() {
  return llvm::IntegerType::getInt8PtrTy(ctx);
}
